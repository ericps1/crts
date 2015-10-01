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

    /// \brief Defines the different types of CE events.
    ///
    /// The different circumstances under which the CE
    /// can be executed are defined here.
    enum Event{
        /// \brief The CE had not been executed for a period
        /// of time as defined by ExtensibleCognitiveRadio::ce_timeout_ms.
        /// It is now executed as a timeout event.
        TIMEOUT = 0,    // event is triggered by a timer
        /// \brief A PHY layer event has caused the execution 
        /// of the CE. Usually this means a frame was received
        /// by the radio.
        PHY,    // event is triggered by the reception of a physical layer frame
    };

    enum FrameType{
        DATA = 0,
        CONTROL,
        UNKNOWN
    };

    // metric struct
    struct metric_s{
        /// \brief Specifies the circumstances under which
        /// the CE was executed.
        ///
        /// When the CE is executed, this value is set according
        /// to the type of event that caused the CE execution,
        /// as specified in ::CE_event_types.
        // Flag for metric type
        ExtensibleCognitiveRadio::Event CE_event;
        ExtensibleCognitiveRadio::FrameType CE_frame;

        // PHY
        int header_valid;
        unsigned char header[8];
        unsigned char* payload;
        int payload_valid;
        unsigned int payload_len;
        unsigned int frame_num;
        framesyncstats_s stats; // stats used by ofdmtxrx object (RSSI, EVM)
        uhd::time_spec_t time_spec;

    };

    // tx parameter struct
    struct tx_parameter_s{
        unsigned int M;                 // number of subcarriers
        unsigned int cp_len;            // cyclic prefix length
        unsigned int taper_len;         // taper length
        unsigned char * p;
        ofdmflexframegenprops_s fgprops;// frame generator properties
        float tx_gain_uhd;
        float tx_gain_soft;
        float tx_freq;
        float tx_rate;
    };
        
    // rx parameter struct
    struct rx_parameter_s{
        unsigned int M;                 // number of subcarriers
        unsigned int cp_len;            // cyclic prefix length
        unsigned int taper_len;         // taper length
        unsigned char * p;
        float rx_gain_uhd;
        float rx_freq;
        float rx_rate;
    };

    // cognitive engine methods
    void set_ce(char * ce); // method to set CE to custom defined subclass
    void start_ce();
    /// \brief Assign a value to ExtensibleCognitiveRadio::ce_timeout_ms.
    void set_ce_timeout_ms(float new_timeout_ms);
    /// \brief Get the current value of ExtensibleCognitiveRadio::ce_timeout_ms 
    float get_ce_timeout_ms();

    struct metric_s CE_metrics; // struct containing metrics used by cognitive engine
    
    // network layer methods
    void set_ip(char *ip);

       // transmitter methods
    void set_tx_freq(float _tx_freq);
    void set_tx_rate(float _tx_rate);
    void set_tx_gain_soft(float _tx_gain_soft);
    void set_tx_gain_uhd(float _tx_gain_uhd);
    void set_tx_antenna(char * _tx_antenna);
    void set_tx_modulation(int mod_scheme);
    void set_tx_crc(int crc_scheme);
    void set_tx_fec0(int fec_scheme);
    void set_tx_fec1(int fec_scheme);
    void set_tx_subcarriers(unsigned int subcarriers);
    void set_tx_subcarrier_alloc(char *subcarrier_alloc);
    void set_tx_cp_len(unsigned int cp_len);
    void set_tx_taper_len(unsigned int taper_len);
    void set_header(unsigned char * _header);
    void increase_tx_mod_order();
    void decrease_tx_mod_order();
    
    float get_tx_freq();
    float get_tx_rate();
    float get_tx_gain_soft();
    float get_tx_gain_uhd();
    char* get_tx_antenna();
    int get_tx_modulation();
    int get_tx_crc();
    int get_tx_fec0();
    int get_tx_fec1();
    unsigned int get_tx_subcarriers();
    void get_tx_subcarrier_alloc(char *p);
    unsigned int get_tx_cp_len();
    unsigned int get_tx_taper_len();
    void get_header(unsigned char *h);
    

    void start_tx();
    void stop_tx();
    void reset_tx();
    
    void transmit_frame(unsigned char * _header,
            unsigned char *  _payload,
            unsigned int     _payload_len);

    // receiver methods
    void set_rx_freq(float _rx_freq);
    void set_rx_rate(float _rx_rate);
    void set_rx_gain_uhd(float _rx_gain_uhd);
    void set_rx_antenna(char * _rx_antenna);
    void set_rx_subcarriers(unsigned int subcarriers);
    void set_rx_subcarrier_alloc(char *subcarrier_alloc);
    void set_rx_cp_len(unsigned int cp_len);
    void set_rx_taper_len(unsigned int taper_len);

    float get_rx_freq();
    float get_rx_rate();
    float get_rx_gain_uhd();
    char* get_rx_antenna();
    unsigned int get_rx_subcarriers();
    void get_rx_subcarrier_alloc(char *p);
    unsigned int get_rx_cp_len();
    unsigned int get_rx_taper_len();
    
    void reset_rx();
    void start_rx();
    void stop_rx();
       
    // methods and variables for printing/logging metrics 
    void print_metrics(ExtensibleCognitiveRadio * CR);
    int print_metrics_flag;
    void log_rx_metrics();
    void log_tx_parameters();
    int log_rx_metrics_flag;
    int log_tx_parameters_flag;
    char rx_log_file[100];
    char tx_log_file[100];
	void reset_log_files();

    // USRP objects accessible to user for now
    uhd::usrp::multi_usrp::sptr usrp_tx;
    uhd::tx_metadata_t metadata_tx;
    
    uhd::usrp::multi_usrp::sptr usrp_rx;
    uhd::rx_metadata_t metadata_rx;
    
private:
    
    // cognitive engine objects
    Cognitive_Engine * CE; // pointer to CE object

    /// \brief The maximum length of time to go
    /// without an event before executing the CE
    /// under a timeout event. In milliseconds.
    ///
    /// The CE is executed every time an event occurs. 
    /// The CE can also be executed if no event has occured
    /// after some period of time. 
    /// This is referred to as a timeout event and this variable
    /// defines the length of the timeout period in milliseconds.
    ///
    /// It can be accessed using ExtensibleCognitiveRadio::set_ce_timeout_ms()
    /// and ExtensibleCognitiveRadio::get_ce_timeout_ms().
    float ce_timeout_ms;

    // variables to enable/disable ce events
    bool ce_phy_events;
    
    // cognitive engine threading objects
    pthread_t CE_process;
    pthread_mutex_t CE_mutex;
    pthread_cond_t CE_execute_sig;    
    friend void * ECR_ce_worker(void *);
    
    // network layer objects
    int tunfd; // virtual network interface

    // receiver properties/objects
    struct rx_parameter_s rx_params;
    ofdmflexframesync fs;           // frame synchronizer object
    unsigned int frame_num;

    // receiver threading objects
    pthread_t rx_process;           // receive thread
    pthread_mutex_t rx_mutex;       // receive mutex
    pthread_cond_t  rx_cond;        // receive condition
    bool rx_running;                // is receiver running? (physical receiver)
    bool rx_thread_running;         // is receiver thread running?
    friend void * ECR_rx_worker(void *);
    
    // receiver callback
    friend int rxCallback(unsigned char *, int, unsigned char *, unsigned int, int,    framesyncstats_s, void *);

    // transmitter properties/objects
    tx_parameter_s tx_params;
    ofdmflexframegen fg;            // frame generator object
    unsigned int fgbuffer_len;      // length of frame generator buffer
    std::complex<float> * fgbuffer; // frame generator output buffer [size: M + cp_len x 1]
    unsigned char tx_header[8];        // header container (must have length 8)
    unsigned int frame_counter;
    
    // transmitter threading objects
    pthread_t tx_process;
    pthread_mutex_t tx_mutex;
    pthread_cond_t tx_cond;
    bool tx_thread_running;
    bool tx_running;
    friend void * ECR_tx_worker(void *);    
    
};

#endif
