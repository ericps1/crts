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

    /// \brief Defines the different types of frames 
    /// that might be received, causing 
    /// Cognitive_Engine::execute() to be called.
    enum FrameType{
        /// \brief The frame contains application
        /// layer data. 
        ///
        /// Generally, most frames received by the radio
        /// will contain application layer data, 
        /// not directly useful to the cognitive engine (CE).
        /// However, the other parameters in 
        /// ExtensibleCognitiveRadio::metric_s
        /// may still contain important information about the 
        /// quality of the received frame, which the 
        /// CE may decide to act on.
        DATA = 0,

        /// \brief The frame was sent explicitly at the
        /// behest of another cognitive engine (CE) in the
        /// network and it contains custom data for use 
        /// by the receiving CE.
        /// 
        /// The handling of ExtensibleCognitiveRadio::DATA 
        /// frames is performed automatically by the
        /// Extensible Cognitive Radio (ECR).
        /// However, the CE may initiate the transmission
        /// of a custom control frame containing 
        /// information to be relayed to another CE in the 
        /// network. 
        /// A custom frame can be sent using 
        /// ExtensibleCognitiveRadio::transmit_frame().
        CONTROL,
        /// \brief The Extensible Cognitve Radio (ECR) is
        /// unable to determine the type of the received frame.
        ///
        /// The received frame was too corrupted to determine
        /// its type. 
        UNKNOWN
    };

    // metric struct
    /// \brief Contains metric information when a 
    /// ExtensibleCognitiveRadio::Event occurs.
    /// This information is made available to the
    /// custom Cognitive_Engine::execute() implementation.
    ///
    /// Whenever an ExtensibleCognitiveRadio::Event occurs, 
    /// an instance of this struct is filled with information
    /// pertinent to the event that may then be used by the 
    /// cognitive engine's (CE's) custom implementation of 
    /// Cognitive_Engine::execute().
    /// 
    /// The most important member of this struct is 
    /// ExtensibleCognitiveRadio::metric_s::CE_event 
    /// which alerts the CE of what type of event 
    /// caused the CE exectution. 
    /// 
    /// The other members of this struct are dependent on the 
    /// event type and will only contain valid information
    /// when the CE has been executed under the corrseponding event.
    /// Otherwise, they should not be accesssed. 
    ///
    /// The valid members under a 
    /// ExtensibleCognitiveRadio::PHY event are:
    /// 
    /// ExtensibleCognitiveRadio::metric_s::CE_frame,
    /// 
    /// ExtensibleCognitiveRadio::metric_s::header_valid,
    /// 
    /// ExtensibleCognitiveRadio::metric_s::header,
    /// 
    /// ExtensibleCognitiveRadio::metric_s::payload,
    /// 
    /// ExtensibleCognitiveRadio::metric_s::payload_valid,
    /// 
    /// ExtensibleCognitiveRadio::metric_s::payload_len,
    /// 
    /// ExtensibleCognitiveRadio::metric_s::frame_num,
    /// 
    /// ExtensibleCognitiveRadio::metric_s::stats, and
    /// 
    /// ExtensibleCognitiveRadio::metric_s::time_spec
    struct metric_s{
        /// \brief Specifies the circumstances under which
        /// the CE was executed.
        ///
        /// When the CE is executed, this value is set according
        /// to the type of event that caused the CE execution,
        /// as specified in ExtensibleCognitiveRadio::Event.
        // Flag for metric type
        ExtensibleCognitiveRadio::Event CE_event;

        // PHY
        /// \brief Specifies the type of frame received as 
        /// defined by ExtensibleCognitiveRadio::FrameType
        ExtensibleCognitiveRadio::FrameType CE_frame;

        /// \brief Indicates whether the \p header of the 
        /// received frame passed error checking tests.
        ///
        /// Derived from 
        /// \c <a href="http://liquidsdr.org/">liquid-dsp</a>. See the
        /// <a href="http://liquidsdr.org/doc/tutorial_ofdmflexframe.html">Liquid Documentation</a>
        /// for more information.
        int header_valid;
        /// \brief The raw header data of the received frame.
        unsigned char header[8];
        /// \brief The raw payload data of the received frame.
        unsigned char* payload;
        /// \brief Indicates whether the \p payload of the 
        /// received frame passed error checking tests.
        ///
        /// Derived from 
        /// \c <a href="http://liquidsdr.org/">liquid-dsp</a>. See the
        /// <a href="http://liquidsdr.org/doc/tutorial_ofdmflexframe.html">Liquid Documentation</a>
        /// for more information.
        int payload_valid;
        /// \brief The number of elements of the \p payload array.
        ///
        /// Equal to the byte length of the \p payload.
        unsigned int payload_len;
        /// \brief The frame number of the received 
        /// ExtensibleCognitiveRadio::DATA frame.
        ///
        /// Each ExtensibleCognitiveRadio::DATA frame transmitted
        /// by the ECR is assigned a number, according to the order
        /// in which it was transmitted.
        unsigned int frame_num;
        /// \brief The statistics of the received frame as
        /// reported by 
        /// \c <a href="http://liquidsdr.org/">liquid-dsp</a>.
        ///
        /// For information about its members, refer to the
        /// <a href="http://liquidsdr.org/doc/framing.html#framing:framesyncstats_s">Liquid Documentation</a>.
        framesyncstats_s stats; // stats used by ofdmtxrx object (RSSI, EVM)
        /// \brief The 
        /// <a href="http://files.ettus.com/manual/classuhd_1_1time__spec__t.html">uhd::time_spec_t</a>
        /// object returned by the 
        /// <a href="http://files.ettus.com/manual/index.html">UHD</a> driver upon reception 
        /// of a complete frame. 
        ///
        /// This serves as a marker to denote at what time the 
        /// end of the frame was received.
        uhd::time_spec_t time_spec;
    };

    // tx parameter struct
    /// \brief Contains parameters defining how
    /// to handle frame transmission.
    struct tx_parameter_s{
        unsigned int M;                 // number of subcarriers
        unsigned int cp_len;            // cyclic prefix length
        unsigned int taper_len;         // taper length
        unsigned char * p;
        /// \brief The properties for the OFDM frame generator from 
        /// <a href="http://liquidsdr.org/">liquid</a>.
        /// 
        /// See 
        /// \todo insert reference to liquid documentation of ofdmflexframegenprops_s
        /// for details.
        /// 
        /// Members of this struct can be accessed with the following functions:
        /// - \p check:
        ///     + ExtensibleCognitiveRadio::set_tx_crc()
        ///     + ExtensibleCognitiveRadio::get_tx_crc().
        /// - \p fec0:
        ///     + ExtensibleCognitiveRadio::set_tx_fec0()
        ///     + ExtensibleCognitiveRadio::get_tx_fec0().
        /// - \p fec1:
        ///     + ExtensibleCognitiveRadio::set_tx_fec1()
        ///     + ExtensibleCognitiveRadio::get_tx_fec1().
        /// - \p mod_scheme:
        ///     + ExtensibleCognitiveRadio::set_tx_modulation()
        ///     + ExtensibleCognitiveRadio::get_tx_modulation().
        ofdmflexframegenprops_s fgprops;// frame generator properties
        /// \brief The value of the hardware gain. In dB. 
        ///
        /// Sets the gain of the hardware amplifier in the transmit chain
        /// of the USRP. 
        /// This value is passed directly to 
        /// <a href="http://files.ettus.com/manual/index.html">UHD</a>.
        ///
        /// It can be accessed with ExtensibleCognitiveRadio::set_tx_gain_uhd()
        /// and ExtensibleCognitiveRadio::get_tx_gain_uhd().
        /// 
        /// Run
        /// 
        ///     $ uhd_usrp_probe
        /// 
        /// for details about the particular gain limits of your USRP device.
        float tx_gain_uhd;
        /// \brief The software gain of the transmitter. In dB. 
        ///
        /// In addition to the hardware gain 
        /// (ExtensibleCognitiveRadio::tx_parameter_s::tx_gain_uhd), 
        /// the gain of the transmission can be adjusted in software by
        /// setting this parameter.  It is converted to a linear factor
        /// and then applied to the frame samples before they are sent to
        /// <a href="http://files.ettus.com/manual/index.html">UHD</a>.
        ///
        /// It can be accessed with ExtensibleCognitiveRadio::set_tx_gain_soft()
        /// and ExtensibleCognitiveRadio::get_tx_gain_soft().
        ///
        /// Note that the values of samples sent to 
        /// <a href="http://files.ettus.com/manual/index.html">UHD</a>
        /// must be between -1 and 1. 
        /// Approaching these limits may cause distortion and exceeding them
        /// will cause clipping. 
        /// Thus it is generally recommended to set this parameter below zero.
        /// \todo See, (insert reference), for example values.
        float tx_gain_soft;
        /// \brief The transmitter frequency in Hertz. 
        ///
        /// It can be accessed with ExtensibleCognitiveRadio::set_tx_freq()
        /// and ExtensibleCognitiveRadio::get_tx_freq().
        ///
        /// This value is passed directly to 
        /// <a href="http://files.ettus.com/manual/index.html">UHD</a>.
        float tx_freq;
        /// \brief The sample rate of the transmitter in samples/second. 
        ///
        /// It can be accessed with ExtensibleCognitiveRadio::set_tx_rate()
        /// and ExtensibleCognitiveRadio::get_tx_rate().
        ///
        /// This value is passed directly to 
        /// <a href="http://files.ettus.com/manual/index.html">UHD</a>.
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
    /// \brief Set the value of ExtensibleCognitiveRadio::tx_parameter_s::tx_freq.
    void set_tx_freq(float _tx_freq);
    /// \brief Set the value of ExtensibleCognitiveRadio::tx_parameter_s::tx_rate.
    void set_tx_rate(float _tx_rate);
    /// \brief Set the value of ExtensibleCognitiveRadio::tx_parameter_s::tx_gain_soft.
    void set_tx_gain_soft(float _tx_gain_soft);
    /// \brief Set the value of ExtensibleCognitiveRadio::tx_parameter_s::tx_gain_uhd.
    void set_tx_gain_uhd(float _tx_gain_uhd);
    void set_tx_antenna(char * _tx_antenna);
    /// \brief Set the value of \p mod_scheme in ExtensibleCognitiveRadio::tx_parameter_s::fgprops.
    void set_tx_modulation(int mod_scheme);
    /// \brief Set the value of \p check in ExtensibleCognitiveRadio::tx_parameter_s::fgprops.
    void set_tx_crc(int crc_scheme);
    /// \brief Set the value of \p fec0 in ExtensibleCognitiveRadio::tx_parameter_s::fgprops.
    void set_tx_fec0(int fec_scheme);
    /// \brief Set the value of \p fec1 in ExtensibleCognitiveRadio::tx_parameter_s::fgprops.
    void set_tx_fec1(int fec_scheme);
    void set_tx_subcarriers(unsigned int subcarriers);
    void set_tx_subcarrier_alloc(char *subcarrier_alloc);
    void set_tx_cp_len(unsigned int cp_len);
    void set_tx_taper_len(unsigned int taper_len);
    void set_header(unsigned char * _header);
    void increase_tx_mod_order();
    void decrease_tx_mod_order();
    
    /// \brief Return the value of ExtensibleCognitiveRadio::tx_parameter_s::tx_freq.
    float get_tx_freq();
    /// \brief Return the value of ExtensibleCognitiveRadio::tx_parameter_s::tx_rate.
    float get_tx_rate();
    /// \brief Return the value of ExtensibleCognitiveRadio::tx_parameter_s::tx_gain_soft.
    float get_tx_gain_soft();
    /// \brief Return the value of ExtensibleCognitiveRadio::tx_parameter_s::tx_gain_uhd.
    float get_tx_gain_uhd();
    char* get_tx_antenna();
    /// \brief Return the value of \p mod_scheme in ExtensibleCognitiveRadio::tx_parameter_s::fgprops.
    int get_tx_modulation();
    /// \brief Return the value of \p check in ExtensibleCognitiveRadio::tx_parameter_s::fgprops.
    int get_tx_crc();
    /// \brief Return the value of \p fec0 in ExtensibleCognitiveRadio::tx_parameter_s::fgprops.
    int get_tx_fec0();
    /// \brief Return the value of \p fec1 in ExtensibleCognitiveRadio::tx_parameter_s::fgprops.
    int get_tx_fec1();
    unsigned int get_tx_subcarriers();
    void get_tx_subcarrier_alloc(char *p);
    unsigned int get_tx_cp_len();
    unsigned int get_tx_taper_len();
    void get_header(unsigned char *h);
    

    void start_tx();
    void stop_tx();
    void reset_tx();
    
    /// \brief Transmit a custom frame.
    ///
    /// The cognitive engine (CE) can initiate transmission
    /// of a custom frame by calling this function.
    /// \p _header must be a pointer to an array
    /// of exactly 8 elements of type 
    /// \c unsigned \c int.
    /// The first byte of \p _header \b must be set to 
    /// ExtensibleCognitiveRadio::CONTROL.
    /// For Example:
    /// @code
    /// ExtensibleCognitiveRadio ECR;
    /// unsigned char myHeader[8];
    /// unsigned char myPayload[20];
    /// myHeader[0] = ExtensibleCognitiveRadio::CONTROL.
    /// ECR.transmit_frame(myHeader, myPayload, 20);
    /// @endcode
    /// \p _payload is an array of \c unsigned \c char
    /// and can be any length. It can contain any data 
    /// as would be useful to the CE.
    ///
    /// \p _payload_len is the number of elements in
    /// \p _payload.
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
