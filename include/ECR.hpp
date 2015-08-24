#ifndef _CR_HPP_
#define _CR_HPP_

#include <stdio.h>
#include <math.h>
#include <complex>
#include <liquid/liquid.h>
//#include <liquid/ofdmtxrx.h>
#include <pthread.h>
#include <uhd/usrp/multi_usrp.hpp>
#include "CE.hpp"

enum CE_event_types{
	ce_timeout = 0,	// event is triggered by a timer
	ce_phy_event,	// event is triggered by the reception of a physical layer frame
};

enum CE_frame_types{
    ce_frame_data = 0,
    ce_frame_control
};

// metric struct
struct metric_s{
	// Flag for metric type
	int CE_event;
    int CE_frame;

	// PHY
	int header_valid;
	unsigned char header[8];
    unsigned char* payload;
	int payload_valid;
    unsigned int payload_len;
    unsigned int packet_id;
	framesyncstats_s stats; // stats used by ofdmtxrx object (RSSI, EVM)
	uhd::time_spec_t time_spec;

};

// thread functions
void * ECR_tx_worker(void * _arg);
void * ECR_rx_worker(void * _arg);
void * ECR_ce_worker(void * _arg);

// function that receives frame from PHY layer
int rxCallback(unsigned char * _header,
		int _header_valid,
		unsigned char * _payload,
		unsigned int _payload_len,
		int _payload_valid,
		framesyncstats_s _stats,
		void * _userdata);

class ExtensibleCognitiveRadio {
public:
	ExtensibleCognitiveRadio();
	~ExtensibleCognitiveRadio();

	// cognitive engine methods
	void set_ce(char * ce); // method to set CE to custom defined subclass
    void start_ce();
	void set_ce_timeout_ms(float new_timeout_ms);
	float get_ce_timeout_ms();

	// cognitive engine objects
    Cognitive_Engine * CE; // pointer to CE object
	struct metric_s CE_metrics; // struct containing metrics used by cognitive engine
	float ce_timeout_ms;

    // variables to enable/disable ce events
	bool ce_phy_events;
	
	// cognitive engine threading objects
    pthread_t CE_process;
    pthread_mutex_t CE_mutex;
    pthread_cond_t CE_execute_sig;	
	friend void * CR_ce_worker(void * _arg);

	// network layer methods
    void set_ip(char *ip);

    // network layer objects
    int tunfd; // virtual network interface

	// transmitter methods
    void set_tx_freq(float _tx_freq);
    float get_tx_freq();
    void set_tx_rate(float _tx_rate);
    void set_tx_gain_soft(float _tx_gain_soft);
    void set_tx_gain_uhd(float _tx_gain_uhd);
    void set_tx_antenna(char * _tx_antenna);
    void reset_tx();
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

    void start_tx();
	void stop_tx();

	void transmit_packet(unsigned char * _header,
			unsigned char *  _payload,
			unsigned int     _payload_len);

    // transmitter properties/objects
    unsigned int M;                 // number of subcarriers
    unsigned int cp_len;            // cyclic prefix length
    unsigned int taper_len;         // taper length
    unsigned char * p;
    ofdmflexframegenprops_s fgprops;// frame generator properties

    ofdmflexframegen fg;            // frame generator object
    std::complex<float> * fgbuffer; // frame generator output buffer [size: M + cp_len x 1]
    unsigned int fgbuffer_len;      // length of frame generator buffer
    float tx_gain;                  // soft transmit gain (linear)
    unsigned char tx_header[8];        // header container (must have length 8)
	uhd::usrp::multi_usrp::sptr usrp_tx;
    uhd::tx_metadata_t metadata_tx;

    unsigned int packet_id;

    // transmitter threading objects
    pthread_t tx_process;
    pthread_mutex_t tx_mutex;
    pthread_cond_t tx_cond;
    bool tx_thread_running;
    bool tx_running;
	friend void * CR_tx_worker(void * _arg);
	
    // receiver methods
    void set_rx_freq(float _rx_freq);
    float get_rx_freq();
    void set_rx_rate(float _rx_rate);
    void set_rx_gain_uhd(float _rx_gain_uhd);
    void set_rx_antenna(char * _rx_antenna);
    void reset_rx();
    void start_rx();
    void stop_rx();
    void set_rx_subcarriers(unsigned int subcarriers);
    void set_rx_subcarrier_alloc(char *subcarrier_alloc);
    void set_rx_cp_len(unsigned int cp_len);
    void set_rx_taper_len(unsigned int taper_len);

    // receiver objects
	ofdmflexframesync fs;           // frame synchronizer object
    uhd::usrp::multi_usrp::sptr usrp_rx;
    uhd::rx_metadata_t metadata_rx;
    unsigned int num_written;

	// receiver threading objects
    pthread_t rx_process;           // receive thread
    pthread_mutex_t rx_mutex;       // receive mutex
    pthread_cond_t  rx_cond;        // receive condition
    bool rx_running;                // is receiver running? (physical receiver)
    bool rx_thread_running;         // is receiver thread running?
    friend void * CR_rx_worker(void * _arg);

    // methods and variables for printing/logging metrics 
	void print_metrics(ExtensibleCognitiveRadio * CR);
	int print_metrics_flag;
	void log_metrics(ExtensibleCognitiveRadio * CR);
	int log_metrics_flag;
	char log_file[30];


private:
};

#endif
