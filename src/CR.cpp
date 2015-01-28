#include<stdio.h>
//#include<iostream>
//#include<fstream>
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
    fgprops.fec0        = LIQUID_FEC_HAMMING128;
    fgprops.fec1        = LIQUID_FEC_NONE;
    fgprops.mod_scheme      = LIQUID_MODEM_QAM256;
    fg = ofdmflexframegen_create(M, cp_len, taper_len, p, &fgprops);

    // allocate memory for frame generator output (single OFDM symbol)
    fgbuffer_len = M + cp_len;
    fgbuffer = (std::complex<float>*) malloc(fgbuffer_len * sizeof(std::complex<float>));

    // create frame synchronizer
    fs = ofdmflexframesync_create(M, cp_len, taper_len, p, rxCallback, (void *)this);

	// create usrp objects
    uhd::device_addr_t dev_addr;
    usrp_tx = uhd::usrp::multi_usrp::make(dev_addr);
    usrp_rx = uhd::usrp::multi_usrp::make(dev_addr);
	
	// initialize default tx values
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

	// create and start rx thread
    rx_running = false;             // receiver is not running initially
    rx_thread_running = true;           // receiver thread IS running initially
    pthread_mutex_init(&rx_mutex, NULL);    // receiver mutex
    pthread_cond_init(&rx_cond,   NULL);    // receiver condition
    pthread_create(&rx_process,   NULL, CR_rx_worker, (void*)this);
	
    // create and start tx thread
    /*tx_running = false; // transmitter is not running initially
    tx_thread_running = true; // transmitter thread IS running initially
    pthread_mutex_init(&tx_mutex, NULL); // transmitter mutex
    pthread_cond_init(&tx_cond, NULL); // transmitter condition
    pthread_create(&tx_process, NULL, CR_tx_worker, (void*)this);
	*/
	
	// Point to CE execute function	
    //CE_execute = &CE_execute_1;

	// Start CE thread
    pthread_mutex_init(&CE_mutex, NULL);
	pthread_cond_init(&CE_execute_sig, NULL);
	pthread_create(&CE_process, NULL, CR_ce_worker, (void*)this);
	

    // Create TUN interface
    /*char tun_name[IFNAMSIZ];
    strcpy(tun_name, "tun1");
    tun_fd = tun_alloc(tun_name, IFF_TUN);
	*/
    // Create TAP interface
    //char tap_name[IFNAMSIZ];
    //strcpy(tap_name, "tap44");
    //tapfd = tun_alloc(tap_name, IFF_TAP);

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

void CognitiveRadio::set_ce(char *ce){

	if(!strcmp(ce, "CE_Example_1")){
		CE = new CE_Example_1();
		printf("Set CE to CE_Example_1\n");
	}
	if(!strcmp(ce, "CE_Example_2")){
		CE = new CE_Example_2();
		printf("Set CE to CE_Example_2\n");
	}
}

void CognitiveRadio::set_ip(char *ip){
	char command[40];
	sprintf(command, "ip addr add %s/24 dev tun0", ip);
	system(command);
	//system("route add default gw 10.0.0.1 tun0");
	system("ip link set dev tun0 up");
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
    //pthread_mutex_lock(&tx_mutex);
    usrp_tx->set_tx_freq(_tx_freq);
    //pthread_mutex_unlock(&tx_mutex);
}

// set transmitter sample rate
void CognitiveRadio::set_tx_rate(float _tx_rate)
{
    //pthread_mutex_lock(&tx_mutex);
    usrp_tx->set_tx_rate(_tx_rate);
    //pthread_mutex_unlock(&tx_mutex);
}

// set transmitter software gain
void CognitiveRadio::set_tx_gain_soft(float _tx_gain_soft)
{
    //pthread_mutex_lock(&tx_mutex);
	tx_gain = powf(10.0f, _tx_gain_soft / 20.0f);
    //pthread_mutex_unlock(&tx_mutex);
}

// set transmitter hardware (UHD) gain
void CognitiveRadio::set_tx_gain_uhd(float _tx_gain_uhd)
{
    //pthread_mutex_lock(&tx_mutex);
    usrp_tx->set_tx_gain(_tx_gain_uhd);
    //pthread_mutex_unlock(&tx_mutex);
}

// set modulation scheme
void CognitiveRadio::set_tx_modulation(int mod_scheme)
{
    //pthread_mutex_lock(&tx_mutex);
    fgprops.mod_scheme = mod_scheme;
    ofdmflexframegen_setprops(fg, &fgprops);
    //pthread_mutex_unlock(&tx_mutex);
}

// decrease modulation order
void CognitiveRadio::decrease_tx_mod_order()
{
	// Check to see if modulation order is already minimized
	if (fgprops.mod_scheme != 1 && fgprops.mod_scheme != 9 && fgprops.mod_scheme != 17 &&
		fgprops.mod_scheme != 25 && fgprops.mod_scheme != 32){
			//pthread_mutex_lock(&tx_mutex);
   		 	fgprops.mod_scheme--;
    		ofdmflexframegen_setprops(fg, &fgprops);
			//pthread_mutex_unlock(&tx_mutex);
	}
}

// increase modulation order
void CognitiveRadio::increase_tx_mod_order()
{
	// check to see if modulation order is already maximized
	if (fgprops.mod_scheme != 8 && fgprops.mod_scheme != 16 && fgprops.mod_scheme != 24 &&
		fgprops.mod_scheme != 31 && fgprops.mod_scheme != 38){		
			//pthread_mutex_lock(&tx_mutex);
   			fgprops.mod_scheme++;
    		ofdmflexframegen_setprops(fg, &fgprops);
    		//pthread_mutex_unlock(&tx_mutex);
	}
}

// set CRC scheme
void CognitiveRadio::set_tx_crc(int crc_scheme){
	//pthread_mutex_lock(&tx_mutex);
    fgprops.check = crc_scheme;
	ofdmflexframegen_setprops(fg, &fgprops);
    //pthread_mutex_unlock(&tx_mutex);
}

// set FEC0
void CognitiveRadio::set_tx_fec0(int fec_scheme)
{
    //pthread_mutex_lock(&tx_mutex);
    fgprops.fec0 = fec_scheme;
    ofdmflexframegen_setprops(fg, &fgprops);
    //pthread_mutex_unlock(&tx_mutex);
}

// set FEC1
void CognitiveRadio::set_tx_fec1(int fec_scheme)
{
    //pthread_mutex_lock(&tx_mutex);
    fgprops.fec1 = fec_scheme;
    ofdmflexframegen_setprops(fg, &fgprops);
    //pthread_mutex_unlock(&tx_mutex);
}

// set number of subcarriers
void CognitiveRadio::set_tx_subcarriers(unsigned int _M)
{
    // destroy frame gen, set cp length, recreate frame gen
	//pthread_mutex_lock(&tx_mutex);
	ofdmflexframegen_destroy(fg);
	M = _M;
	fg = ofdmflexframegen_create(M, cp_len, taper_len, p, &fgprops);
	//pthread_mutex_unlock(&tx_mutex);
}

// set subcarrier allocation
void CognitiveRadio::set_tx_subcarrier_alloc(char *_p)
{
    // destroy frame gen, set cp length, recreate frame gen
	//pthread_mutex_lock(&tx_mutex);
	ofdmflexframegen_destroy(fg);
	memcpy(p, _p, M);
	fg = ofdmflexframegen_create(M, cp_len, taper_len, p, &fgprops);
	//pthread_mutex_unlock(&tx_mutex);
}

// set cp_len
void CognitiveRadio::set_tx_cp_len(unsigned int _cp_len)
{
    // destroy frame gen, set cp length, recreate frame gen
	//pthread_mutex_lock(&tx_mutex);
	ofdmflexframegen_destroy(fg);
	cp_len = _cp_len;
	fg = ofdmflexframegen_create(M, cp_len, taper_len, p, &fgprops);
	//pthread_mutex_unlock(&tx_mutex);
}

// set taper_len
void CognitiveRadio::set_tx_taper_len(unsigned int _taper_len)
{
	// destroy frame gen, set cp length, recreate frame gen
	//pthread_mutex_lock(&tx_mutex);
	ofdmflexframegen_destroy(fg);
	taper_len = _taper_len;
	fg = ofdmflexframegen_create(M, cp_len, taper_len, p, &fgprops);
	//pthread_mutex_unlock(&tx_mutex);
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
void CognitiveRadio::set_rx_subcarriers(unsigned int _M)
{
    // stop rx, destroy frame sync, set subcarriers, recreate frame sync
	stop_rx();
	usleep(1.0);
	ofdmflexframesync_destroy(fs);
	M = _M;
	fs = ofdmflexframesync_create(M, cp_len, taper_len, p, rxCallback, (void*)this);
	start_rx();
}

// set subcarrier allocation
void CognitiveRadio::set_rx_subcarrier_alloc(char *_p)
{
    // destroy frame gen, set cp length, recreate frame gen
	stop_rx();
	usleep(1.0);
	ofdmflexframesync_destroy(fs);
	memcpy(p, _p, M);
	fs = ofdmflexframesync_create(M, cp_len, taper_len, p, rxCallback, (void*)this);
	start_rx();
}

// set cp_len
void CognitiveRadio::set_rx_cp_len(unsigned int _cp_len)
{
	// destroy frame gen, set cp length, recreate frame gen
	stop_rx();
	usleep(1.0);
	ofdmflexframesync_destroy(fs);
	cp_len = _cp_len;
	fs = ofdmflexframesync_create(M, cp_len, taper_len, p, rxCallback, (void*)this);
	start_rx();
}

// set taper_len
void CognitiveRadio::set_rx_taper_len(unsigned int _taper_len)
{
    // destroy frame gen, set cp length, recreate frame gen
	stop_rx();
	usleep(1.0);
	ofdmflexframesync_destroy(fs);
	taper_len = _taper_len;
	fs = ofdmflexframesync_create(M, cp_len, taper_len, p, rxCallback, (void*)this);
	start_rx();
}

// start receiver
void CognitiveRadio::start_rx()
{
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
    pthread_cond_wait(&(CR->rx_cond), &(CR->rx_mutex));
    
	// unlock the mutex
    pthread_mutex_unlock(&(CR->rx_mutex));
	// condition given; check state: run or exit
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
    // print payload
	//for (unsigned int i=0; i<_payload_len; i++)
	//	printf("%c\n", _payload[i]);
	
	// typecast user argument as CR object
    CognitiveRadio * CR = (CognitiveRadio*)_userdata;
		
    // if using PHY layer ARQ
    //if (CR->arq)
	// Check if ACK/NACK and update ARQ state
    //	if (*_header == ACK || *_header == NACK) CR->arq.update();

    // Store metrics and signal CE thread if using PHY layer metrics
    if (CR->PHY_metrics){
		CR->CE_metrics.header_valid = _header_valid;
		CR->CE_metrics.payload_valid = _payload_valid;
		CR->CE_metrics.stats = _stats;

		// Signal CE thread
		//printf("Signaling CE thread to execute CE\n");
		pthread_mutex_lock(&CR->CE_mutex);
		pthread_cond_signal(&CR->CE_execute_sig);
		pthread_mutex_unlock(&CR->CE_mutex);

		// Print metrics if required
		if(CR->print_metrics_flag)
			CR->print_metrics(CR);

		// Pass metrics to controller if required
		// Log metrics locally if required
		if(CR->log_metrics_flag)
			CR->log_metrics(CR);
    }
    // Pass payload to tun interface
    //int nwrite = cwrite(CR->tun_fd, (char*)_payload, (int)_payload_len);
    //if(nwrite != (int)_payload_len) 
	//	printf("Number of bytes written to TUN interface not equal to payload length\n"); 

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
		pthread_cond_wait(&(CR->tx_cond), &(CR->tx_mutex));
		// unlock the mutex
		//printf("tx_worker unlocking mutex\n");
		//pthread_mutex_unlock(&(CR->tx_mutex));
		// condition given; check state: run or exit
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
				close(CR->tun_fd);
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
				payload_len);
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
    //printf("CE thread has begun\n");
	CognitiveRadio *CR = (CognitiveRadio *) _arg; 
    // Infinite loop
    while (true){
        // Wait for signal from receiver
		pthread_mutex_lock(&CR->CE_mutex);
		pthread_cond_wait(&CR->CE_execute_sig, &CR->CE_mutex);
		// execute CE
		printf("Executing CE\n");
		CR->CE->execute((void*)CR);
    	pthread_mutex_unlock(&CR->CE_mutex);
    }
    printf("ce_worker exiting thread\n");
    pthread_exit(NULL);

}

void CognitiveRadio::print_metrics(CognitiveRadio * CR){
	printf("\n---------------------------------------------------------\n");
	printf("Received packet metrics:      Received Packet Parameters:\n");
	printf("---------------------------------------------------------\n");
	printf("Header Valid:     %-6i      Modulation Scheme:   %u\n", 
		CR->CE_metrics.header_valid, CR->CE_metrics.stats.mod_scheme);
	printf("Payload Valid:    %-6i      Modulation bits/sym: %u\n", 
		CR->CE_metrics.payload_valid, CR->CE_metrics.stats.mod_bps);
	printf("EVM:              %-8.2f    Check:               %u\n", 
		CR->CE_metrics.stats.evm, CR->CE_metrics.stats.check);
	printf("RSSI:             %-8.2f    Inner FEC:           %u\n", 
		CR->CE_metrics.stats.rssi, CR->CE_metrics.stats.fec0);
	printf("Frequency Offset: %-8.2f    Outter FEC:          %u\n", 
		CR->CE_metrics.stats.cfo, CR->CE_metrics.stats.fec1);
}

void CognitiveRadio::log_metrics(CognitiveRadio * CR){
	// create string of actual file location
	char file_name[100];
	strcpy(file_name, "/users/ericps1/crts/logs/");
	strcat(file_name, CR->log_file);
	
	// open file, append metrics, and close
	FILE * file;
	file = fopen(file_name, "ab");	
	fwrite(&CR->CE_metrics, sizeof(struct metric_s), 1, file); 
	fclose(file);
}

// specific implementation of cognitive engine (will be moved to external file in the future)
void CE_execute_1(void * _arg){
	//printf("Executing CE\n");
	CognitiveRadio * CR = (CognitiveRadio *)_arg;
	
	// Modify modulation based on EVM
	if (!CR->CE_metrics.payload_valid){
		//printf("Payload was invalid (%i) decreasing modulation order\n", CR->CE_metrics.payload_valid);
		CR->decrease_tx_mod_order();
	}
}











