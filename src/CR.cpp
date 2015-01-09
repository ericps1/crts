#include<stdlib.h>
#include<stdio.h>
#include<net/if.h>
#include<linux/if_tun.h>
#include<math.h>
#include<complex>
#include<liquid/liquid.h>
#include<pthread.h>
#include"CR.hpp"
#include"TUN.hpp"

// Constructor
CognitiveRadio::CognitiveRadio(/*string with name of CE_execute function*/){

    // set internal properties
    M = 64;
    cp_len = 16;
    taper_len = 4; 
    p = NULL;   // subcarrier allocation (default)
    
    // create frame generator
    ofdmflexframegenprops_init_default(&fgprops);
    fgprops.check       = LIQUID_CRC_32;
    fgprops.fec0        = LIQUID_FEC_NONE;
    fgprops.fec1        = LIQUID_FEC_HAMMING128;
    fgprops.mod_scheme      = LIQUID_MODEM_QPSK;
    fg = ofdmflexframegen_create(M, cp_len, taper_len, p, &fgprops);

    // allocate memory for frame generator output (single OFDM symbol)
    fgbuffer_len = M + cp_len;
    fgbuffer = (std::complex<float>*) malloc(fgbuffer_len * sizeof(std::complex<float>));

    // create frame synchronizer
    fs = ofdmflexframesync_create(M, cp_len, taper_len, p, rxCallback, (void *)this);
    // TODO: create buffer

    // create usrp objects
    uhd::device_addr_t dev_addr;
    usrp_tx = uhd::usrp::multi_usrp::make(dev_addr);
    usrp_rx = uhd::usrp::multi_usrp::make(dev_addr);

    // initialize default tx values
    set_tx_freq(462.0e6f);
    set_tx_rate(500e3);
    set_tx_gain_soft(-12.0f);
    set_tx_gain_uhd(40.0f);

    // initialize default rx values
    set_rx_freq(462.0e6f);
    set_rx_rate(500e3);
    set_rx_gain_uhd(20.0f);

    // reset transceiver
    reset_tx();
    reset_rx();

    // create and start rx thread
    rx_running = false;             // receiver is not running initially
    rx_thread_running = true;           // receiver thread IS running initially
    pthread_mutex_init(&rx_mutex, NULL);    // receiver mutex
    pthread_cond_init(&rx_cond,   NULL);    // receiver condition
    pthread_create(&rx_process,   NULL, CR_rx_worker, (void*)this);

    // create and start tx thread
    tx_running = false; // transmitter is not running initially
    tx_thread_running = true; // transmitter thread IS running initially
    pthread_mutex_init(&tx_mutex, NULL); // transmitter mutex
    pthread_cond_init(&tx_cond, NULL); // transmitter condition
    pthread_create(&tx_process, NULL, CR_tx_worker, (void*)this);
	
    // Create TUN interface
    char tun_name[IFNAMSIZ];
    strcpy(tun_name, "tun1");
    tun_fd = tun_alloc(tun_name, IFF_TUN);  /* tun interface */

    // Create TAP interface
    //char tap_name[IFNAMSIZ];
    //strcpy(tap_name, "tap44");
    //tapfd = tun_alloc(tap_name, IFF_TAP);  /* tap interface */

    // Point to CE execute function	
    CE_execute = *CE_execute_1;

    // Start CE thread
    pthread_create(&CE_process, NULL, CR_ce_worker, (void*)this);
}

// Destructor
CognitiveRadio::~CognitiveRadio(){

    printf("waiting for process to finish...\n");

    // ensure reciever thread is not running
    if (rx_running) stop_rx();

    // signal condition (tell rx worker to continue)
    printf("destructor signaling condition...\n");
    rx_thread_running = false;
    pthread_cond_signal(&rx_cond);

    printf("destructor joining rx thread...\n");
    void * exit_status;
    pthread_join(rx_process, &exit_status);

    // destroy threading objects
    printf("destructor destroying mutex...\n");
    pthread_mutex_destroy(&rx_mutex);
    printf("destructor destroying condition...\n");
    pthread_cond_destroy(&rx_cond);
    
    // TODO: output debugging file
    if (debug_enabled)
    ofdmflexframesync_debug_print(fs, "ofdmtxrx_framesync_debug.m");

    printf("destructor destroying other objects...\n");
    // destroy framing objects
    ofdmflexframegen_destroy(fg);
    ofdmflexframesync_destroy(fs);

    // free other allocated arrays
    free(fgbuffer);

    // Stop transceiver
    stop_tx();
    stop_rx();

    // Terminate CE thread
    pthread_cancel(CE_process);

}

////////////////////////////////////////////////////////////////////////
// Transmit methods
////////////////////////////////////////////////////////////////////////

// start transmitter
void CognitiveRadio::start_tx()
{
    printf("usrp tx start\n");
    // set tx running flag
    tx_running = true;
    // signal condition (tell tx worker to start)
    pthread_cond_signal(&tx_cond);
}

// stop transmitter
void CognitiveRadio::stop_tx()
{
    printf("usrp tx stop\n");
    // set rx running flag
    tx_running = false;
}

// reset transmitter objects and buffers
void CognitiveRadio::reset_tx()
{
    ofdmflexframegen_reset(fg);
}

// set transmitter frequency
void CognitiveRadio::set_tx_freq(float _tx_freq)
{
    pthread_mutex_lock(&tx_mutex);
    usrp_tx->set_tx_freq(_tx_freq);
    pthread_mutex_unlock(&tx_mutex);
}

// set transmitter sample rate
void CognitiveRadio::set_tx_rate(float _tx_rate)
{
    pthread_mutex_lock(&tx_mutex);
    usrp_tx->set_tx_rate(_tx_rate);
    pthread_mutex_unlock(&tx_mutex);
}

// set transmitter software gain
void CognitiveRadio::set_tx_gain_soft(float _tx_gain_soft)
{
    pthread_mutex_lock(&tx_mutex);
    tx_gain = powf(10.0f, _tx_gain_soft / 20.0f);
    pthread_mutex_unlock(&tx_mutex);
}

// set transmitter hardware (UHD) gain
void CognitiveRadio::set_tx_gain_uhd(float _tx_gain_uhd)
{
    pthread_mutex_lock(&tx_mutex);
    usrp_tx->set_tx_gain(_tx_gain_uhd);
    pthread_mutex_unlock(&tx_mutex);
}

// set modulation scheme
void CognitiveRadio::set_tx_modulation(int mod_scheme)
{
    pthread_mutex_lock(&tx_mutex);
    fgprops.mod_scheme = mod_scheme;
    pthread_mutex_unlock(&tx_mutex);

}

// set FEC0
void CognitiveRadio::set_tx_fec0(int fec_scheme)
{
    pthread_mutex_lock(&tx_mutex);
    fgprops.fec0 = fec_scheme;
    pthread_mutex_unlock(&tx_mutex);

}

// set FEC1
void CognitiveRadio::set_tx_fec1(int fec_scheme)
{
    pthread_mutex_lock(&tx_mutex);
    fgprops.fec1 = fec_scheme;
    pthread_mutex_unlock(&tx_mutex);

}

// set number of subcarriers
void CognitiveRadio::set_tx_subcarriers(unsigned int subcarriers)
{
    // ofdmflexframegen destroy
    // ofdmflexframegen create
}

// set subcarrier allocation
void CognitiveRadio::set_tx_subcarrier_alloc(char *subcarrier_alloc)
{
    // ofdmflexframegen destroy
    // ofdmflexframegen create
}

// set cp_len
void CognitiveRadio::set_tx_cp_len(unsigned int cp_len)
{
    // ofdmflexframegen destroy
    // ofdmflexframegen create
}

// set taper_len
void CognitiveRadio::set_tx_taper_len(unsigned int taper_len)
{
    // ofdmflexframegen destroy
    // ofdmflexframegen create
}

// update payload data on a particular channel
void CognitiveRadio::transmit_packet(unsigned char * _header,
                   unsigned char * _payload,
                   unsigned int    _payload_len)/*,
                   int         _mod,
                   int         _fec0,
                   int         _fec1)*/
{
    // set up the metadta flags
    metadata_tx.start_of_burst = false; // never SOB when continuous
    metadata_tx.end_of_burst   = false; // 
    metadata_tx.has_time_spec  = false; // set to false to send immediately
    //TODO: flush buffers

    // fector buffer to send data to device
    std::vector<std::complex<float> > usrp_buffer(fgbuffer_len);

    // set properties
    /*fgprops.mod_scheme  = _mod;
    fgprops.fec0    = _fec0;
    fgprops.fec1    = _fec1;
    ofdmflexframegen_setprops(fg, &fgprops);*/

    // assemble frame
    ofdmflexframegen_assemble(fg, _header, _payload, _payload_len);

    // generate a single OFDM frame
    bool last_symbol=false;
    unsigned int i;
    while (!last_symbol) {

    // generate symbol
    last_symbol = ofdmflexframegen_writesymbol(fg, fgbuffer);

    // copy symbol and apply gain
    for (i=0; i<fgbuffer_len; i++)
        usrp_buffer[i] = fgbuffer[i] * tx_gain;

    // send samples to the device
    usrp_tx->get_device()->send(
        &usrp_buffer.front(), usrp_buffer.size(),
        metadata_tx,
        uhd::io_type_t::COMPLEX_FLOAT32,
        uhd::device::SEND_MODE_FULL_BUFF
    );

    } // while loop

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
void CognitiveRadio::set_rx_freq(float _rx_freq)
{
    usrp_rx->set_rx_freq(_rx_freq);
}

// set receiver sample rate
void CognitiveRadio::set_rx_rate(float _rx_rate)
{
    usrp_rx->set_rx_rate(_rx_rate);
}

// set receiver hardware (UHD) gain
void CognitiveRadio::set_rx_gain_uhd(float _rx_gain_uhd)
{
    usrp_rx->set_rx_gain(_rx_gain_uhd);
}

// set receiver antenna
void CognitiveRadio::set_rx_antenna(char * _rx_antenna)
{
    usrp_rx->set_rx_antenna(_rx_antenna);
}

// reset receiver objects and buffers
void CognitiveRadio::reset_rx()
{
    ofdmflexframesync_reset(fs);
}

// set number of subcarriers
void CognitiveRadio::set_rx_subcarriers(unsigned int subcarriers)
{
    // ofdmflexframegen destroy
    // ofdmflexframegen create
}

// set subcarrier allocation
void CognitiveRadio::set_rx_subcarrier_alloc(char *subcarrier_alloc)
{
    // ofdmflexframegen destroy
    // ofdmflexframegen create
}

// set cp_len
void CognitiveRadio::set_rx_cp_len(unsigned int cp_len)
{
    // ofdmflexframegen destroy
    // ofdmflexframegen create
}

// set taper_len
void CognitiveRadio::set_rx_taper_len(unsigned int taper_len)
{
    // ofdmflexframegen destroy
    // ofdmflexframegen create
}


// start receiver
void CognitiveRadio::start_rx()
{
    printf("usrp rx start\n");
    // set rx running flag
    rx_running = true;

    // tell device to start
    usrp_rx->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);

    // signal condition (tell rx worker to start)
    pthread_cond_signal(&rx_cond);
}

// stop receiver
void CognitiveRadio::stop_rx()
{
    printf("usrp rx stop\n");
    // set rx running flag
    rx_running = false;

    // tell device to stop
    usrp_rx->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
}

// receiver worker thread
void * CR_rx_worker(void * _arg)
{
    // type cast input argument as ofdmtxrx object
    CognitiveRadio * CR = (CognitiveRadio*) _arg;

    // set up receive buffer
    const size_t max_samps_per_packet = CR->usrp_rx->get_device()->get_max_recv_samps_per_packet();
    std::vector<std::complex<float> > buffer(max_samps_per_packet);

    // receiver metadata object
    uhd::rx_metadata_t md;

    while (CR->rx_thread_running) {
    // wait for signal to start; lock mutex
    pthread_mutex_lock(&(CR->rx_mutex));

    // this function unlocks the mutex and waits for the condition;
    // once the condition is set, the mutex is again locked
    printf("rx_worker waiting for condition...\n");
    //int status =
    pthread_cond_wait(&(CR->rx_cond), &(CR->rx_mutex));
    printf("rx_worker received condition\n");

    // unlock the mutex
    printf("rx_worker unlocking mutex\n");
    pthread_mutex_unlock(&(CR->rx_mutex));

    // condition given; check state: run or exit
    printf("rx_worker running...\n");
    if (!CR->rx_running) {
        printf("rx_worker finished\n");
        break;
    }

    // run receiver
    while (CR->rx_running) {

        // grab data from device
        //printf("rx_worker waiting for samples...\n");
        size_t num_rx_samps = CR->usrp_rx->get_device()->recv(
        &buffer.front(), buffer.size(), md,
        uhd::io_type_t::COMPLEX_FLOAT32,
        uhd::device::RECV_MODE_ONE_PACKET
        );
        //printf("rx_worker processing samples...\n");

        // ignore error codes for now
#if 0
        // 'handle' the error codes
        switch(md.error_code){
        case uhd::rx_metadata_t::ERROR_CODE_NONE:
        case uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:
        break;

        default:
        std::cerr << "Error code: " << md.error_code << std::endl;
        std::cerr << "Unexpected error on recv, exit test..." << std::endl;
        //return 1;
        //std::cerr << "rx_worker exiting prematurely" << std::endl;
        //pthread_exit(NULL);
        }
#endif

        // push data through frame synchronizer
        // TODO : use arbitrary resampler?
        unsigned int j;
        for (j=0; j<num_rx_samps; j++) {
        // grab sample from usrp buffer
        std::complex<float> usrp_sample = buffer[j];

        // push resulting samples through synchronizer
        ofdmflexframesync_execute(CR->fs, &usrp_sample, 1);
        }

    } // while rx_running
    printf("rx_worker finished running\n");

    } // while true
    
    //
    printf("rx_worker exiting thread\n");
    pthread_exit(NULL);
}

// function to handle packets received by the CR object
int rxCallback(unsigned char * _header,
	int _header_valid,
	unsigned char * _payload,
	unsigned int _payload_len,
	int _payload_valid,
	framesyncstats_s _stats,
	void * _userdata)
{
    printf("Packet received!\n");
    printf("Payload:");
    for(unsigned int i=0; i<_payload_len; i++) printf("%i\n", _payload[i]);

    // typecast user argument as CR object
    CognitiveRadio * CR = (CognitiveRadio*)_userdata;
		
    // if using PHY layer ARQ
    //if (CR->arq)
	// Check if ACK/NACK and update ARQ state
    //	if (*_header == ACK || *_header == NACK) CR->arq.update();

    // Store metrics and signal CE thread if using PHY layer metrics
    if (CR->PHY_metrics){
	CR->CE_metrics.payload_valid = _payload_valid;
	CR->CE_metrics.stats = _stats;

	// Signal CE thread
	pthread_cond_signal(&CR->CE_execute_sig);

	// Pass metrics to controller if required
	// Log metrics locally if required
    }
    // Pass payload to tap interface
    int nwrite = cwrite(CR->tun_fd, (char*)_payload, (int)_payload_len);
    if(nwrite != (int)_payload_len) printf("Number of bytes written to TUN interface not equal to payload length\n"); 

    // Transmit acknowledgement if using PHY ARQ
    /*unsigned char *ACK;
    pthread_mutex_lock(&(CR->tx_mutex));
    CR->transmit_packet(ACK,
	ACK,
	0,
	LIQUID_MODEM_QPSK, // might want to use set schemes
	LIQUID_FEC_NONE,
	LIQUID_FEC_HAMMING128);
    pthread_mutex_unlock(&(CR->tx_mutex));
    */
    return 0;
}


// transmitter worker thread
void * CR_tx_worker(void * _arg)
{
    // type cast input argument as CR object
    CognitiveRadio * CR = (CognitiveRadio*)_arg;

    // set up transmit buffer
    int buffer_len = 1024;
    unsigned char buffer[buffer_len];
    unsigned char *payload;
    unsigned int payload_len;
    unsigned char header[1];
    int nread;

    while (CR->tx_thread_running) {
	// wait for signal to start; lock mutex
	//pthread_mutex_lock(&(CR->tx_mutex));
	// this function unlocks the mutex and waits for the condition;
	// once the condition is set, the mutex is again locked
	printf("tx_worker waiting for condition...\n");
	//int status =
	pthread_cond_wait(&(CR->tx_cond), &(CR->tx_mutex));
	printf("tx_worker received condition\n");
	// unlock the mutex
	//printf("tx_worker unlocking mutex\n");
	//pthread_mutex_unlock(&(CR->tx_mutex));
	// condition given; check state: run or exit
	printf("tx_worker running...\n");
	if (!CR->tx_running) {
	    printf("tx_worker finished\n");
	break;
	}
	// run transmitter
	while (CR->tx_running) {
	    // grab data from TAP interface
	    nread = read(CR->tun_fd, buffer, sizeof(buffer));
	    if (nread < 0) {
		perror("Reading from interface");
		//close(tun_fd);
		exit(1);
	    }

	    // set header (needs to be modified)
	    header[1] = 1;

	    // resize to packet length if necessary
	    payload = buffer;
	    payload_len = nread;
	 
	    // transmit packet
	    pthread_mutex_lock(&(CR->tx_mutex));
	    CR->transmit_packet(header,
		payload,
		payload_len);/*,
		LIQUID_FEC_NONE,
		LIQUID_FEC_HAMMING128,
		LIQUID_MODEM_QPSK);*/
		pthread_mutex_unlock(&(CR->tx_mutex));

        } // while tx_running
        printf("tx_worker finished running\n");
    } // while true
    //
    printf("tx_worker exiting thread\n");
    pthread_exit(NULL);
}

// main loop of CE
void * CR_ce_worker(void *_arg){
    CognitiveRadio *CR = (CognitiveRadio *) _arg; 
    // Infinite loop
    while (true){
        // Wait for signal from receiver
	pthread_cond_wait(&CR->CE_execute_sig, &CR->CE_mutex);

	// Lock mutex until finished processing
	pthread_mutex_lock(&CR->CE_mutex);

	// execute CE
	CR->CE_execute(&CR->CE_metrics);

	// unlock mutex
    	pthread_mutex_unlock(&CR->CE_mutex);
    }
    printf("ce_worker exiting thread\n");
    pthread_exit(NULL);

}

// specific implementation of cognitive engine (will be moved to external file in the future)
void CE_execute_1(struct metric_s * CE_metrics){
	printf("Executing CE\n");
	// Make decisions based on metrics
	// Call CR methods to modify behavior
	// Any CE algorithms requiring receive information other than the feedback
	// from the ofdmtxrx object will need to stop_rx() first
}


