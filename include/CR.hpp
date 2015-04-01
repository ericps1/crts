#ifndef _CR_HPP_
#define _CR_HPP_

#include <stdio.h>
#include <math.h>
#include <complex>
#include <liquid/liquid.h>
#include <liquid/ofdmtxrx.h>
#include <pthread.h>
#include <uhd/usrp/multi_usrp.hpp>
#include "CE.hpp"

/*enum headers{
	ACK,	// Acknowledgement
	NACK,	// Negative-acknowledgement
	PRACH,  // Physical random access request
	PCH,    // Paging channel
	DCH		// Data 
};*/

// metric struct
struct metric_s{
	// Flag for metric type
	/*enum layer{
		PHY,
		MAC,
		NET,
		TRP,
		APP
	};*/

	// PHY
	int header_valid;
	unsigned char header[8];
	int payload_valid;
	framesyncstats_s stats; // stats used by ofdmtxrx object (RSSI, EVM)
	uhd::time_spec_t time_spec;

	// MAC

	// NET

	// TRP

	// APP
};

// thread functions
void * CR_tx_worker(void * _arg);
void * CR_rx_worker(void * _arg);
void * CR_ce_worker(void * _arg);

// function that defines cognitive engine
void CE_execute_1(void * _arg);

// function that receives frame from PHY layer
int rxCallback(unsigned char * _header,
		int _header_valid,
		unsigned char * _payload,
		unsigned int _payload_len,
		int _payload_valid,
		framesyncstats_s _stats,
		void * _userdata);

class CognitiveRadio {
public:
	CognitiveRadio(/*string with name of CE_execute function*/);
	~CognitiveRadio();

	void start_tx();
	void stop_tx();

    // 
    // transmitter methods
    //
    void set_tx_freq(float _tx_freq);
    float get_tx_freq();
    void set_tx_rate(float _tx_rate);
    void set_tx_gain_soft(float _tx_gain_soft);
    void set_tx_gain_uhd(float _tx_gain_uhd);
    void set_tx_antenna(char * _tx_antenna);
    void reset_tx();
    // added transmitter methods
    void set_tx_modulation(int mod_scheme);
    void increase_tx_mod_order();
	void decrease_tx_mod_order();
	void set_tx_crc(int crc_scheme);
	void set_tx_fec0(int fec_scheme);
    void set_tx_fec1(int fec_scheme);
    void set_tx_subcarriers(unsigned int subcarriers);
    void set_tx_subcarrier_alloc(char *subcarrier_alloc);
    void set_tx_cp_len(unsigned int cp_len);
    void set_tx_taper_len(unsigned int taper_len);
    void set_header(unsigned char * _header);

    // 
    // receiver methods
    //
    void set_rx_freq(float _rx_freq);
    float get_rx_freq();
    void set_rx_rate(float _rx_rate);
    void set_rx_gain_uhd(float _rx_gain_uhd);
    void set_rx_antenna(char * _rx_antenna);
    void reset_rx();
    void start_rx();
    void stop_rx();
    // added receiver methods
    void set_rx_subcarriers(unsigned int subcarriers);
    void set_rx_subcarrier_alloc(char *subcarrier_alloc);
    void set_rx_cp_len(unsigned int cp_len);
    void set_rx_taper_len(unsigned int taper_len);

	// print/log metrics methods and variables
	void print_metrics(CognitiveRadio * CR);
	int print_metrics_flag;
	void log_metrics(CognitiveRadio * CR);
	int log_metrics_flag;
	char log_file[30];

    // specify tx worker method as friend function so that it may
    // gain acess to private members of the class
    friend void * CR_tx_worker(void * _arg);
    friend void * CR_rx_worker(void * _arg);
    friend void * CR_ce_worker(void * _arg);

    // OFDM properties
    unsigned int M;                 // number of subcarriers
    unsigned int cp_len;            // cyclic prefix length
    unsigned int taper_len;         // taper length
    unsigned char * p;
    ofdmflexframegenprops_s fgprops;// frame generator properties

    // transmitter objects
    ofdmflexframegen fg;            // frame generator object
    std::complex<float> * fgbuffer; // frame generator output buffer [size: M + cp_len x 1]
    unsigned int fgbuffer_len;      // length of frame generator buffer
    float tx_gain;                  // soft transmit gain (linear)
    unsigned char tx_header[8];        // header container (must have length 8)

    void transmit_packet(unsigned char * _header,
			unsigned char *  _payload,
			unsigned int     _payload_len);
			
	// receiver objects
    ofdmflexframesync fs;           // frame synchronizer object
    pthread_t rx_process;           // receive thread
    pthread_mutex_t rx_mutex;       // receive mutex
    pthread_cond_t  rx_cond;        // receive condition
    bool rx_running;                // is receiver running? (physical receiver)
    bool rx_thread_running;         // is receiver thread running?
    bool debug_enabled;             // is debugging enabled?

    // RF objects and properties
    uhd::usrp::multi_usrp::sptr usrp_tx;
    uhd::usrp::multi_usrp::sptr usrp_rx;
    uhd::tx_metadata_t metadata_tx;
	uhd::rx_metadata_t metadata_rx;

	float max_gain_tx;
	float max_gain_rx;

    // pointer to CE execute function
    //void (*CE_execute)(void * _arg);

	// pointer to CE object
	Cognitive_Engine * CE;
	// method to set CE to custom defined subclass
	void set_ce(char * ce);

    // struct containing metrics used by cognitive engine
    struct metric_s CE_metrics;

    // flag for feedback metrics for each layer
    bool PHY_metrics;
    bool MAC_metrics;
    bool NET_metrics;
    bool TRP_metrics;
    bool APP_metrics;

    // Connection state object?

    // ARQ protocol object
    //ARQ arq;

    // TUN file descriptor
    int tun_fd;

	void set_ip(char *ip);


    // TX thread, mutex, signal, and thread states
    pthread_t tx_process;
    pthread_mutex_t tx_mutex;
    pthread_cond_t tx_cond;
    bool tx_thread_running;
    bool tx_running;
	
    // CE thread, mutex, and signal
    pthread_t CE_process;
    pthread_mutex_t CE_mutex;
    pthread_cond_t CE_execute_sig;

    // CE thread timeout length in milliseconds
    float timeout_length_ms;
    void set_timeout_length_ms(float new_timeout_length_ms);
    float get_timeout_length_ms();



private:
};

#endif
