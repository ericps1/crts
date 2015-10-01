#include <stdio.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <math.h>
#include <complex>
#include <liquid/liquid.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <sys/time.h>
#include "ECR.hpp"
#include "TUN.hpp"

#define DEBUG 0
#if DEBUG == 1
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

// Constructor
ExtensibleCognitiveRadio::ExtensibleCognitiveRadio(){

    // Set initial timeout value for executing CE
    ce_timeout_ms = 1000;

    // set internal properties
    tx_params.M = 64;
    tx_params.cp_len = 16;
    tx_params.taper_len = 4; 
    tx_params.p = NULL;   // subcarrier allocation (default)
    rx_params.M = 64;
    rx_params.cp_len = 16;
    rx_params.taper_len = 4; 
    rx_params.p = NULL;   // subcarrier allocation (default)

    // Initialize header to all zeros
    memset(tx_header, 0, sizeof(tx_header));

    // enable physical layer events for ce
    ce_phy_events = true;
    
    // create frame generator
    ofdmflexframegenprops_init_default(&tx_params.fgprops);
    tx_params.fgprops.check       = LIQUID_CRC_32;
    tx_params.fgprops.fec0        = LIQUID_FEC_HAMMING128;
    tx_params.fgprops.fec1        = LIQUID_FEC_NONE;
    tx_params.fgprops.mod_scheme      = LIQUID_MODEM_QAM4;
    fg = ofdmflexframegen_create(tx_params.M, tx_params.cp_len, tx_params.taper_len, tx_params.p, &tx_params.fgprops);

    // allocate memory for frame generator output (single OFDM symbol)
    fgbuffer_len = tx_params.M + tx_params.cp_len;
    fgbuffer = (std::complex<float>*) malloc(fgbuffer_len * sizeof(std::complex<float>));

    // create frame synchronizer
    fs = ofdmflexframesync_create(rx_params.M, rx_params.cp_len, rx_params.taper_len, rx_params.p, rxCallback, (void *)this);

    // create usrp objects
    uhd::device_addr_t dev_addr;
    usrp_tx = uhd::usrp::multi_usrp::make(dev_addr);
    usrp_rx = uhd::usrp::multi_usrp::make(dev_addr);
    
    // Create TUN interface
    dprintf("Creating tun interface\n");
    char tun_name[IFNAMSIZ];
    strcpy(tun_name, "tun0");
    tunfd = tun_alloc(tun_name, IFF_TUN);

    dprintf("Bringing up tun interface\n");
    system("ip link set dev tun0 up");
    usleep(1e6);

    // create and start rx thread
    dprintf("Starting rx thread...\n");
    rx_running = false;             // receiver is not running initially
    rx_thread_running = true;           // receiver thread IS running initially
    pthread_mutex_init(&rx_mutex, NULL);    // receiver mutex
    pthread_cond_init(&rx_cond,   NULL);    // receiver condition
    pthread_create(&rx_process,   NULL, ECR_rx_worker, (void*)this);
    
    // create and start tx thread
    frame_num = 0;
    tx_running = false; // transmitter is not running initially
    tx_thread_running = true; // transmitter thread IS running initially
    pthread_mutex_init(&tx_mutex, NULL); // transmitter mutex
    pthread_cond_init(&tx_cond, NULL); // transmitter condition
    pthread_create(&tx_process, NULL, ECR_tx_worker, (void*)this);    
    
    // Start CE thread
    dprintf("Starting CE thread...\n");
    pthread_mutex_init(&CE_mutex, NULL);
    pthread_cond_init(&CE_execute_sig, NULL);
    
    pthread_create(&CE_process, NULL, ECR_ce_worker, (void*)this);
    
    // initialize default tx values
    dprintf("Initializing USRP settings...\n");
    set_tx_freq(460.0e6f);
    set_tx_rate(500e3);
    set_tx_gain_soft(-12.0f);
    set_tx_gain_uhd(0.0f);

    // initialize default rx values
    set_rx_freq(460.0e6f);
    set_rx_rate(500e3);
    set_rx_gain_uhd(0.0f);
    
    // reset transceiver
    reset_tx();
    reset_rx();

    dprintf("Finished creating ECR\n");
}

// Destructor
ExtensibleCognitiveRadio::~ExtensibleCognitiveRadio(){

    // undo modifications to network interface
    system("route del -net 10.0.0.0 netmask 255.255.255.0 dev tun0");
    system("ip link set dev tun0 down");
    
    dprintf("waiting for process to finish...\n");

    // ensure reciever thread is not running
    if (rx_running) stop_rx();

    // signal condition (tell rx worker to continue)
    dprintf("destructor signaling condition...\n");
    rx_thread_running = false;
    pthread_cond_signal(&rx_cond);

    dprintf("destructor joining rx thread...\n");
    void * exit_status;
    pthread_join(rx_process, &exit_status);

    // destroy threading objects
    dprintf("destructor destroying mutex...\n");
    pthread_mutex_destroy(&rx_mutex);
    dprintf("destructor destroying condition...\n");
    pthread_cond_destroy(&rx_cond);
    
    dprintf("destructor destroying other objects...\n");
    // destroy framing objects
    ofdmflexframegen_destroy(fg);
    ofdmflexframesync_destroy(fs);

    // Stop transceiver
    stop_tx();
    stop_rx();

    // Terminate CE thread
    pthread_cancel(CE_process);
}

///////////////////////////////////////////////////////////////////////
// Cognitive engine methods
///////////////////////////////////////////////////////////////////////

void ExtensibleCognitiveRadio::set_ce(char *ce){
//EDIT START FLAG
    if(!strcmp(ce, "CE_DSA"))
        CE = new CE_DSA();
    if(!strcmp(ce, "CE_Example"))
        CE = new CE_Example();
    if(!strcmp(ce, "CE_FEC"))
        CE = new CE_FEC();
    if(!strcmp(ce, "CE_Hopper"))
        CE = new CE_Hopper();
    if(!strcmp(ce, "CE_DSA_PU"))
        CE = new CE_DSA_PU();
    if(!strcmp(ce, "CE_AMC"))
        CE = new CE_AMC();
//EDIT END FLAG
}

void ExtensibleCognitiveRadio::start_ce(){
    // signal condition for the ce to start listening for events of interest
    pthread_cond_signal(&CE_execute_sig);
}

void ExtensibleCognitiveRadio::set_ce_timeout_ms(float new_timeout_ms){
    ce_timeout_ms = new_timeout_ms;
}

float ExtensibleCognitiveRadio::get_ce_timeout_ms(){
    return ce_timeout_ms;
}

///////////////////////////////////////////////////////////////////////
// Network methods
///////////////////////////////////////////////////////////////////////

// set the ip address for the virtual network interface
void ExtensibleCognitiveRadio::set_ip(char *ip){
    char command[50];
    sprintf(command, "ip addr add %s/24 dev tun0", ip);
    system(command);
    system("route add -net 10.0.0.0 netmask 255.255.255.0 dev tun0");
}

////////////////////////////////////////////////////////////////////////
// Transmit methods
////////////////////////////////////////////////////////////////////////

// start transmitter
void ExtensibleCognitiveRadio::start_tx()
{
    // set tx running flag
    tx_running = true;
    // signal condition (tell tx worker to start)
    pthread_cond_signal(&tx_cond);
}

// stop transmitter
void ExtensibleCognitiveRadio::stop_tx()
{
    // set rx running flag
    tx_running = false;
}

// reset transmitter objects and buffers
void ExtensibleCognitiveRadio::reset_tx()
{
    ofdmflexframegen_reset(fg);
}

// set transmitter frequency
void ExtensibleCognitiveRadio::set_tx_freq(float _tx_freq)
{
    tx_params.tx_freq = _tx_freq;
    pthread_mutex_lock(&tx_mutex);
    usrp_tx->set_tx_freq(_tx_freq);
    pthread_mutex_unlock(&tx_mutex);
}

// get transmitter frequency
float ExtensibleCognitiveRadio::get_tx_freq()
{
    return tx_params.tx_freq;
}

// set transmitter sample rate
void ExtensibleCognitiveRadio::set_tx_rate(float _tx_rate)
{
    tx_params.tx_rate = _tx_rate;
    pthread_mutex_lock(&tx_mutex);
    usrp_tx->set_tx_rate(_tx_rate);
    pthread_mutex_unlock(&tx_mutex);
}

// get transmitter sample rate
float ExtensibleCognitiveRadio::get_tx_rate()
{
    return tx_params.tx_rate;    
}

// set transmitter software gain
void ExtensibleCognitiveRadio::set_tx_gain_soft(float _tx_gain_soft)
{
    tx_params.tx_gain_soft = _tx_gain_soft;
}

// get transmitter software gain
float ExtensibleCognitiveRadio::get_tx_gain_soft()
{
    return tx_params.tx_gain_soft;
}

// set transmitter hardware (UHD) gain
void ExtensibleCognitiveRadio::set_tx_gain_uhd(float _tx_gain_uhd)
{
    tx_params.tx_gain_uhd = _tx_gain_uhd;
    pthread_mutex_lock(&tx_mutex);
    usrp_tx->set_tx_gain(_tx_gain_uhd);
    pthread_mutex_unlock(&tx_mutex);
}

// get transmitter hardware (UHD) gain
float ExtensibleCognitiveRadio::get_tx_gain_uhd()
{
    return tx_params.tx_gain_uhd;
}

// set modulation scheme
void ExtensibleCognitiveRadio::set_tx_modulation(int mod_scheme)
{
    pthread_mutex_lock(&tx_mutex);
    tx_params.fgprops.mod_scheme = mod_scheme;
    ofdmflexframegen_setprops(fg, &tx_params.fgprops);
    pthread_mutex_unlock(&tx_mutex);
}

// get modulation scheme
int ExtensibleCognitiveRadio::get_tx_modulation()
{
    return tx_params.fgprops.mod_scheme;
}

// decrease modulation order
void ExtensibleCognitiveRadio::decrease_tx_mod_order()
{    
    unsigned int mod = tx_params.fgprops.mod_scheme;
    
    // Check to see if modulation order is already minimized
    if (mod != 1 && mod != 9 && mod != 17 && mod != 25 && mod != 32){
            pthread_mutex_lock(&tx_mutex);
                tx_params.fgprops.mod_scheme--;
            ofdmflexframegen_setprops(fg, &tx_params.fgprops);
            pthread_mutex_unlock(&tx_mutex);
    }
}

// increase modulation order
void ExtensibleCognitiveRadio::increase_tx_mod_order()
{
    unsigned int mod = tx_params.fgprops.mod_scheme;
    
    // check to see if modulation order is already maximized
    if (mod != 8 && mod != 16 && mod != 24 && mod != 31 && mod != 38){        
            pthread_mutex_lock(&tx_mutex);
               tx_params.fgprops.mod_scheme++;
            ofdmflexframegen_setprops(fg, &tx_params.fgprops);
            pthread_mutex_unlock(&tx_mutex);
    }
}

// set CRC scheme
void ExtensibleCognitiveRadio::set_tx_crc(int crc_scheme){
    pthread_mutex_lock(&tx_mutex);
    tx_params.fgprops.check = crc_scheme;
    ofdmflexframegen_setprops(fg, &tx_params.fgprops);
    pthread_mutex_unlock(&tx_mutex);
}

// get CRC scheme
int ExtensibleCognitiveRadio::get_tx_crc(){
    return tx_params.fgprops.check;    
}

// set FEC0
void ExtensibleCognitiveRadio::set_tx_fec0(int fec_scheme)
{
    pthread_mutex_lock(&tx_mutex);
    tx_params.fgprops.fec0 = fec_scheme;
    ofdmflexframegen_setprops(fg, &tx_params.fgprops);
    pthread_mutex_unlock(&tx_mutex);
}

// get FEC0
int ExtensibleCognitiveRadio::get_tx_fec0()
{
    return tx_params.fgprops.fec0; 
}

// set FEC1
void ExtensibleCognitiveRadio::set_tx_fec1(int fec_scheme)
{
    pthread_mutex_lock(&tx_mutex);
    tx_params.fgprops.fec1 = fec_scheme;
    ofdmflexframegen_setprops(fg, &tx_params.fgprops);
    pthread_mutex_unlock(&tx_mutex);
}

// get FEC1
int ExtensibleCognitiveRadio::get_tx_fec1()
{
    return tx_params.fgprops.fec1; 
}

// set number of subcarriers
void ExtensibleCognitiveRadio::set_tx_subcarriers(unsigned int _M)
{
    // destroy frame gen, set cp length, recreate frame gen
    pthread_mutex_lock(&tx_mutex);
    ofdmflexframegen_destroy(fg);
    tx_params.M = _M;
    fg = ofdmflexframegen_create(tx_params.M, tx_params.cp_len, tx_params.taper_len, tx_params.p, &tx_params.fgprops);
    pthread_mutex_unlock(&tx_mutex);
}

// get number of subcarriers
unsigned int ExtensibleCognitiveRadio::get_tx_subcarriers()
{
    return tx_params.M;    
}

// set subcarrier allocation
void ExtensibleCognitiveRadio::set_tx_subcarrier_alloc(char *_p)
{
    // destroy frame gen, set cp length, recreate frame gen
    pthread_mutex_lock(&tx_mutex);
    ofdmflexframegen_destroy(fg);
    memcpy(tx_params.p, _p, tx_params.M);
    fg = ofdmflexframegen_create(tx_params.M, tx_params.cp_len, tx_params.taper_len, tx_params.p, &tx_params.fgprops);
    pthread_mutex_unlock(&tx_mutex);
}

// get subcarrier allocation
void ExtensibleCognitiveRadio::get_tx_subcarrier_alloc(char *p)
{
    memcpy(p, tx_params.p, tx_params.M);
}

// set cp_len
void ExtensibleCognitiveRadio::set_tx_cp_len(unsigned int _cp_len)
{
    // destroy frame gen, set cp length, recreate frame gen
    pthread_mutex_lock(&tx_mutex);
    ofdmflexframegen_destroy(fg);
    tx_params.cp_len = _cp_len;
    fg = ofdmflexframegen_create(tx_params.M, tx_params.cp_len, tx_params.taper_len, tx_params.p, &tx_params.fgprops);
    pthread_mutex_unlock(&tx_mutex);
}

// get cp_len
unsigned int ExtensibleCognitiveRadio::get_tx_cp_len()
{
    return tx_params.cp_len;    
}

// set taper_len
void ExtensibleCognitiveRadio::set_tx_taper_len(unsigned int _taper_len)
{
    // destroy frame gen, set cp length, recreate frame gen
    pthread_mutex_lock(&tx_mutex);
    ofdmflexframegen_destroy(fg);
    tx_params.taper_len = _taper_len;
    fg = ofdmflexframegen_create(tx_params.M, tx_params.cp_len, tx_params.taper_len, tx_params.p, &tx_params.fgprops);
    pthread_mutex_unlock(&tx_mutex);
}

// get taper_len
unsigned int ExtensibleCognitiveRadio::get_tx_taper_len()
{
    return tx_params.taper_len;
}

// set header data (must have length 8)
void ExtensibleCognitiveRadio::set_header(unsigned char * _header)
{
    pthread_mutex_lock(&tx_mutex);
    for(int i=0; i<8; i++)
        tx_header[i] = _header[i];
    pthread_mutex_unlock(&tx_mutex);
}

// get header data
void ExtensibleCognitiveRadio::get_header(unsigned char *h)
{
    memcpy(tx_header, h, 8*sizeof(unsigned char)); 
}

// transmit a frame
void ExtensibleCognitiveRadio::transmit_frame(unsigned char * _header,
                   unsigned char * _payload,
                   unsigned int    _payload_len)
{
    // set up the metadta flags
    metadata_tx.start_of_burst = false; // never SOB when continuous
    metadata_tx.end_of_burst   = false; // 
    metadata_tx.has_time_spec  = false; // set to false to send immediately
    //TODO: flush buffers

    // vector buffer to send data to device
    std::vector<std::complex<float> > usrp_buffer(fgbuffer_len);

    // assemble frame
    ofdmflexframegen_assemble(fg, _header, _payload, _payload_len);

    float tx_gain_soft_lin = powf(10.0f, tx_params.tx_gain_soft / 20.0f);

    if(log_tx_parameters_flag){
        //printf("\nLogging transmit parameters\n\n");
        log_tx_parameters();
    }

    // generate a single OFDM frame
    bool last_symbol=false;
    unsigned int i;
    pthread_mutex_lock(&tx_mutex);
    while (!last_symbol) {

        // generate symbol
        last_symbol = ofdmflexframegen_writesymbol(fg, fgbuffer);

        // copy symbol and apply gain
        for (i=0; i<fgbuffer_len; i++)
            usrp_buffer[i] = fgbuffer[i] * tx_gain_soft_lin;
    
        // send samples to the device
        usrp_tx->get_device()->send(
            &usrp_buffer.front(), usrp_buffer.size(),
            metadata_tx,
            uhd::io_type_t::COMPLEX_FLOAT32,
            uhd::device::SEND_MODE_FULL_BUFF
        );

    } // while loop
    pthread_mutex_unlock(&tx_mutex);

    // send a few extra samples to the device
    // NOTE: this seems necessary to preserve last OFDM symbol in
    //       frame from corruption
    usrp_tx->get_device()->send(
    &usrp_buffer.front(), usrp_buffer.size(),
    metadata_tx,
    uhd::io_type_t::COMPLEX_FLOAT32,
    uhd::device::SEND_MODE_FULL_BUFF
    );
    
    // send a mini EOB packet
    metadata_tx.start_of_burst = false;
    metadata_tx.end_of_burst   = true;

    usrp_tx->get_device()->send("", 0, metadata_tx,
    uhd::io_type_t::COMPLEX_FLOAT32,
    uhd::device::SEND_MODE_FULL_BUFF
    );

}

/////////////////////////////////////////////////////////////////////
// Receiver methods
/////////////////////////////////////////////////////////////////////

// set receiver frequency
void ExtensibleCognitiveRadio::set_rx_freq(float _rx_freq)
{
    rx_params.rx_freq = _rx_freq;
    usrp_rx->set_rx_freq(_rx_freq);
}

// set receiver frequency
float ExtensibleCognitiveRadio::get_rx_freq()
{
    return rx_params.rx_freq;
}

// set receiver sample rate
void ExtensibleCognitiveRadio::set_rx_rate(float _rx_rate)
{
    rx_params.rx_rate = _rx_rate;
    usrp_rx->set_rx_rate(_rx_rate);
}

// get receiver sample rate
float ExtensibleCognitiveRadio::get_rx_rate()
{
    return rx_params.rx_rate;
}

// set receiver hardware (UHD) gain
void ExtensibleCognitiveRadio::set_rx_gain_uhd(float _rx_gain_uhd)
{
    rx_params.rx_gain_uhd = _rx_gain_uhd;
    usrp_rx->set_rx_gain(_rx_gain_uhd);
}

// get receiver hardware (UHD) gain
float ExtensibleCognitiveRadio::get_rx_gain_uhd()
{
    return rx_params.rx_gain_uhd;
}

// set receiver antenna
void ExtensibleCognitiveRadio::set_rx_antenna(char * _rx_antenna)
{
    usrp_rx->set_rx_antenna(_rx_antenna);
}

// reset receiver objects and buffers
void ExtensibleCognitiveRadio::reset_rx()
{
    ofdmflexframesync_reset(fs);
}

// set number of subcarriers
void ExtensibleCognitiveRadio::set_rx_subcarriers(unsigned int _M)
{
    // stop rx, destroy frame sync, set subcarriers, recreate frame sync
    stop_rx();
    usleep(1.0);
    ofdmflexframesync_destroy(fs);
    rx_params.M = _M;
    fs = ofdmflexframesync_create(rx_params.M, rx_params.cp_len, rx_params.taper_len, rx_params.p, rxCallback, (void*)this);
    start_rx();
}

// get number of subcarriers
unsigned int ExtensibleCognitiveRadio::get_rx_subcarriers()
{
    return rx_params.M;
}

// set subcarrier allocation
void ExtensibleCognitiveRadio::set_rx_subcarrier_alloc(char *_p)
{
    // destroy frame gen, set cp length, recreate frame gen
    stop_rx();
    usleep(1.0);
    ofdmflexframesync_destroy(fs);
    memcpy(rx_params.p, _p, rx_params.M);
    fs = ofdmflexframesync_create(rx_params.M, rx_params.cp_len, rx_params.taper_len, rx_params.p, rxCallback, (void*)this);
    start_rx();
}

// get subcarrier allocation
void ExtensibleCognitiveRadio::get_rx_subcarrier_alloc(char *p)
{
    memcpy(p, rx_params.p, rx_params.M);
}


// set cp_len
void ExtensibleCognitiveRadio::set_rx_cp_len(unsigned int _cp_len)
{
    // destroy frame gen, set cp length, recreate frame gen
    stop_rx();
    usleep(1.0);
    ofdmflexframesync_destroy(fs);
    rx_params.cp_len = _cp_len;
    fs = ofdmflexframesync_create(rx_params.M, rx_params.cp_len, rx_params.taper_len, rx_params.p, rxCallback, (void*)this);
    start_rx();
}

// get cp_len
unsigned int ExtensibleCognitiveRadio::get_rx_cp_len()
{
    return rx_params.cp_len;    
}

// set taper_len
void ExtensibleCognitiveRadio::set_rx_taper_len(unsigned int _taper_len)
{
    // destroy frame gen, set cp length, recreate frame gen
    stop_rx();
    usleep(1.0);
    ofdmflexframesync_destroy(fs);
    rx_params.taper_len = _taper_len;
    fs = ofdmflexframesync_create(rx_params.M, rx_params.cp_len, rx_params.taper_len, rx_params.p, rxCallback, (void*)this);
    start_rx();
}

// get taper_len
unsigned int ExtensibleCognitiveRadio::get_rx_taper_len()
{
    return rx_params.taper_len;
}

// start receiver
void ExtensibleCognitiveRadio::start_rx()
{
    // set rx running flag
    rx_running = true;

    // tell device to start
    usrp_rx->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);

    // signal condition (tell rx worker to start)
    pthread_cond_signal(&rx_cond);
}

// stop receiver
void ExtensibleCognitiveRadio::stop_rx()
{
    // set rx running flag
    rx_running = false;

    // tell device to stop
    usrp_rx->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
}

// receiver worker thread
void * ECR_rx_worker(void * _arg)
{
    // type cast input argument as ofdmtxrx object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio*) _arg;

    // set up receive buffer
    const size_t max_samps_per_packet = ECR->usrp_rx->get_device()->get_max_recv_samps_per_packet();
    std::vector<std::complex<float> > buffer(max_samps_per_packet);

    while (ECR->rx_thread_running) {
        // wait for signal to start; lock mutex
        pthread_mutex_lock(&(ECR->rx_mutex));

        // this function unlocks the mutex and waits for the condition;
        // once the condition is set, the mutex is again locked
        pthread_cond_wait(&(ECR->rx_cond), &(ECR->rx_mutex));
    
        // unlock the mutex
        pthread_mutex_unlock(&(ECR->rx_mutex));
        // condition given; check state: run or exit
        if (!ECR->rx_running) {
            dprintf("rx_worker finished\n");
            break;
        }

        // run receiver
        while (ECR->rx_running) {

            // grab data from device
            //printf("rx_worker waiting for samples...\n");
            size_t num_rx_samps = ECR->usrp_rx->get_device()->recv(
                &buffer.front(), buffer.size(), ECR->metadata_rx,
                uhd::io_type_t::COMPLEX_FLOAT32,
                uhd::device::RECV_MODE_ONE_PACKET
            );

            // push data through frame synchronizer
            unsigned int j;
            for (j=0; j<num_rx_samps; j++) {
            // grab sample from usrp buffer
            std::complex<float> usrp_sample = buffer[j];

            // push resulting samples through synchronizer
            ofdmflexframesync_execute(ECR->fs, &usrp_sample, 1);
            }
        
        } // while rx_running
        dprintf("rx_worker finished running\n");

    } // while true
    
    dprintf("rx_worker exiting thread\n");
    pthread_exit(NULL);
}

// function to handle frames received by the ECR object
int rxCallback(unsigned char * _header,
    int _header_valid,
    unsigned char * _payload,
    unsigned int _payload_len,
    int _payload_valid,
    framesyncstats_s _stats,
    void * _userdata)
{
    // typecast user argument as ECR object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio*)_userdata;
        
    // Store metrics and signal CE thread if using PHY layer metrics
    if (ECR->ce_phy_events){
        ECR->CE_metrics.header_valid = _header_valid;
        int j;
        for (j=0; j<8; j++)
        {
            ECR->CE_metrics.header[j] = _header[j];
        }
        ECR->CE_metrics.frame_num = (_header[1] << 8 | _header[2]);
        ECR->CE_metrics.stats = _stats;
        ECR->CE_metrics.time_spec = ECR->metadata_rx.time_spec;
        ECR->CE_metrics.payload_valid = _payload_valid;
        if(_payload_valid)
        {
            ECR->CE_metrics.payload_len = _payload_len;
            ECR->CE_metrics.payload = new unsigned char[_payload_len];
            unsigned int k;
            for(k = 0; k < _payload_len; k++)
            {
                ECR->CE_metrics.payload[k] = _payload[k];
            }
        }
        else
        {
            ECR->CE_metrics.payload_len = 0;
            ECR->CE_metrics.payload = NULL;
        }

        // Signal CE thread
        pthread_mutex_lock(&ECR->CE_mutex);
        ECR->CE_metrics.CE_event = ExtensibleCognitiveRadio::PHY;        // set event type to phy once mutex is locked
        if(_header_valid)
        {
            if('d' == _header[0])
            {
                ECR->CE_metrics.CE_frame = ExtensibleCognitiveRadio::DATA;
            }
            else
            {
                ECR->CE_metrics.CE_frame = ExtensibleCognitiveRadio::CONTROL;
            }
        }
        else
        {
            ECR->CE_metrics.CE_frame = ExtensibleCognitiveRadio::UNKNOWN;
        }
        pthread_cond_signal(&ECR->CE_execute_sig);
        pthread_mutex_unlock(&ECR->CE_mutex);

        // Print metrics if required
        if(ECR->print_metrics_flag)
            ECR->print_metrics(ECR);

        // Pass metrics to controller if required
        // Log metrics locally if required
        if(ECR->log_rx_metrics_flag)
            ECR->log_rx_metrics();
    
    }

    // print payload
    dprintf("\nReceived Payload:\n");
    for (unsigned int i=0; i<_payload_len; i++)
        dprintf("%c", _payload[i]);
    dprintf("\n");

    char payload[_payload_len];
    for(unsigned int i=0; i<_payload_len; i++)
        payload[i] = _payload[i];

    int nwrite = 0;
    if(_payload_valid){
        if('d' == _header[0])
        {
            // Pass payload to tun interface
            dprintf("Passing payload to tun interface\n");
            nwrite = cwrite(ECR->tunfd, payload, (int)_payload_len);
            if(nwrite != (int)_payload_len) 
                printf("Number of bytes written to TUN interface not equal to payload length\n"); 
            else
                ECR->frame_counter++;
        }
    }

    return 0;
}


// transmitter worker thread
void * ECR_tx_worker(void * _arg)
{
    // type cast input argument as ECR object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio*)_arg;

    // set up transmit buffer
    int buffer_len = 1024;
    unsigned char buffer[buffer_len];
    unsigned char *payload;
    unsigned int payload_len;
    int nread;

    while (ECR->tx_thread_running) {
        // wait for signal to start 
        pthread_cond_wait(&(ECR->tx_cond), &(ECR->tx_mutex));
        // unlock the mutex
        pthread_mutex_unlock(&(ECR->tx_mutex));
        
        // condition given; check state: run or exit
        if (!ECR->tx_running) {
            printf("tx_worker finished\n");
        break;
        }
        // run transmitter
        while (ECR->tx_running) {
            memset(buffer, 0, buffer_len);
            
            // grab data from TUN interface
            dprintf("Reading from tun interface\n");
            nread = cread(ECR->tunfd, (char*)buffer, buffer_len);
            if (nread < 0) {
                printf("Error reading from interface");
                close(ECR->tunfd);
                exit(1);
            }
            
            payload = buffer;
            payload_len = nread;
     
            // transmit frame
            dprintf("Buffer read from tun interface:\n");
            for(int i=0; i<buffer_len; i++)
                dprintf("%c", buffer[i]);
            dprintf("\n");

            dprintf("Transmitting frame\n");    
            ECR->tx_header[0] = 'd';
            ECR->tx_header[1] = (ECR->frame_num >> 8) & 0xff;
            ECR->tx_header[2] = (ECR->frame_num) & 0xff;
            ECR->frame_num++;
            ECR->transmit_frame(ECR->tx_header,
                payload,
                payload_len);
        } // while tx_running
        printf("tx_worker finished running\n");
    } // while true
    //
    printf("tx_worker exiting thread\n");
    pthread_exit(NULL);
}

// main loop of CE
void * ECR_ce_worker(void *_arg){
    ExtensibleCognitiveRadio *ECR = (ExtensibleCognitiveRadio *) _arg; 
    
    struct timeval time_now;
    double timeout_ns;
    double timeout_spart;
    double timeout_nspart;
    struct timespec timeout;

    // wait for signal to start ce executio the first signal should be sent by start_ce()
    // every signal thereafter will be triggered by some event the ce is interested in
    pthread_mutex_lock(&ECR->CE_mutex);
    pthread_cond_wait(&ECR->CE_execute_sig, &ECR->CE_mutex);
    pthread_mutex_unlock(&ECR->CE_mutex);

    // Infinite loop
    while (true){

        // Get current time of day
        gettimeofday(&time_now, NULL);

        // Calculate timeout time in nanoseconds
        timeout_ns = (double)time_now.tv_usec*1e3 + (double)time_now.tv_sec*1e9 + ECR->ce_timeout_ms*1e6;
        // Convert timeout time to s and ns parts
        timeout_nspart = modf(timeout_ns/1e9, &timeout_spart);
        // Put timeout into timespec struct
        timeout.tv_sec = (long int)timeout_spart;
        timeout.tv_nsec = (long int)(timeout_nspart*1e9);
    
        // Wait for signal from receiver
        pthread_mutex_lock(&ECR->CE_mutex);
        if(ETIMEDOUT == pthread_cond_timedwait(&ECR->CE_execute_sig, &ECR->CE_mutex, &timeout))
            ECR->CE_metrics.CE_event = ExtensibleCognitiveRadio::TIMEOUT;
        
        // execute CE
        ECR->CE->execute((void*)ECR);
        pthread_mutex_unlock(&ECR->CE_mutex);
    }
    printf("ce_worker exiting thread\n");
    pthread_exit(NULL);

}

void ExtensibleCognitiveRadio::print_metrics(ExtensibleCognitiveRadio * ECR){
    printf("\n---------------------------------------------------------\n");
    if(ECR->CE_metrics.header_valid)
        printf("Received Frame %u metrics:      Received Frame Parameters:\n", ECR->CE_metrics.frame_num);
    else
        printf("Received Frame ? metrics:      Received Frame Parameters:\n");
    printf("---------------------------------------------------------\n");
    printf("Header Valid:     %-6i      Modulation Scheme:   %s\n", 
        ECR->CE_metrics.header_valid, modulation_types[ECR->CE_metrics.stats.mod_scheme].name);
        // See liquid soruce: src/modem/src/modem_utilities.c
        // for definition of modulation_types
    printf("Payload Valid:    %-6i      Modulation bits/sym: %u\n", 
        ECR->CE_metrics.payload_valid, ECR->CE_metrics.stats.mod_bps);
    printf("EVM:              %-8.2f    Check:               %s\n", 
        ECR->CE_metrics.stats.evm, crc_scheme_str[ECR->CE_metrics.stats.check][0]);
        // See liquid soruce: src/fec/src/crc.c
        // for definition of crc_scheme_str
    printf("RSSI:             %-8.2f    Inner FEC:           %s\n", 
        ECR->CE_metrics.stats.rssi, fec_scheme_str[ECR->CE_metrics.stats.fec0][0]);
    printf("Frequency Offset: %-8.2f    Outter FEC:          %s\n", 
        ECR->CE_metrics.stats.cfo, fec_scheme_str[ECR->CE_metrics.stats.fec1][0]);
        // See liquid soruce: src/fec/src/fec.c
        // for definition of fec_scheme_str
}

void ExtensibleCognitiveRadio::log_rx_metrics(){
    
    // open file, append metrics, and close
    std::ofstream log_fstream;
    log_fstream.open(rx_log_file, std::ofstream::out|std::ofstream::binary|std::ofstream::app);
    if (log_fstream.is_open())
    {
        log_fstream.write((char*)&CE_metrics, sizeof(struct metric_s));
    }
    else
    {
        std::cerr<<"Error opening log file:"<<rx_log_file<<std::endl;
    }

    log_fstream.close();
}

void ExtensibleCognitiveRadio::log_tx_parameters(){
    
    // update current time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    // open file, append parameters, and close
    std::ofstream log_fstream;
    log_fstream.open(tx_log_file, std::ofstream::out|std::ofstream::binary|std::ofstream::app);
    if (log_fstream.is_open())
    {
        log_fstream.write((char*)&tv, sizeof(tv));
        log_fstream.write((char*)&tx_params, sizeof(struct tx_parameter_s));
    }
    else
    {
        std::cerr<<"Error opening log file:"<<tx_log_file<<std::endl;
    }

    log_fstream.close();
}

void ExtensibleCognitiveRadio::reset_log_files(){

	if (log_rx_metrics_flag){
        std::ofstream log_fstream;
		log_fstream.open(rx_log_file, std::ofstream::out | std::ofstream::trunc);
		if(log_fstream.is_open())
			log_fstream.close();
        else
			printf("Error opening rx log file: %s\n", rx_log_file);
	}

	if (log_tx_parameters_flag){
        std::ofstream log_fstream;
		log_fstream.open(tx_log_file, std::ofstream::out | std::ofstream::trunc);
		if(log_fstream.is_open())
			log_fstream.close();
        else
			printf("Error opening tx log file: %s\n", tx_log_file);
	}
}

















