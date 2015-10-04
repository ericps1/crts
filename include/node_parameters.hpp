#ifndef _H_NODE_PARAMS_
#define _H_NODE_PARAMS_

#include<string>
#include<liquid/liquid.h>

enum type{
    CR = 0,        // user equipment node type
    interferer        // interferer node type
};

enum cr_type{
    python = 0, //third party python radios
    ecr      //Radios created using ECR
};

enum duplex{
    FDD = 0,        // frequency division duplexing
    TDD,        // time division duplexing (not implemented)
    HD            // half-duplex
};

enum interference_type{
    CW= 0,       // continuous-wave interference
    AWGN,       // additive white gaussian noise interference
    GMSK,     // gaussian minimum-shift keying inteference
    RRC,     // root-raised cosine interference (as in WCDMA)
    OFDM     // orthogonal frequency division multiplexing interference
};


enum tx_freq_hop_type{
    NONE = 0, 
    ALTERNATING,
    SWEEP,
    RANDOM
};


struct node_parameters{
    // general
    int type;
    int cr_type;
    char python_file[100];
    char arguments[20][50];
    int num_arguments;
    char CORNET_IP[20];
    char CRTS_IP[20];
    char TARGET_IP[20];
    char CE[30];
    int print_metrics;
    int log_rx_metrics;
    int log_tx_parameters;
    int log_CRTS_rx_data;
    char rx_log_file[100];
    char tx_log_file[100];
    char CRTS_rx_log_file[100];
    float ce_timeout_ms;
    int generate_octave_logs;
    int generate_python_logs;

	// USRP settings
    float rx_freq;
    float rx_rate;
    float rx_gain;
    float tx_freq;
    float tx_rate;
    float tx_gain;

    // liquid OFDM settings
    int duplex;
    float tx_gain_soft;
    int rx_subcarriers;
	int rx_cp_len;
	int rx_taper_len;
	int tx_subcarriers;
	int tx_cp_len;
	int tx_taper_len;
	int tx_modulation;
    int tx_crc;
    int tx_fec0;
    int tx_fec1;
	float tx_delay_us;
    
    // interferer only
    int   interference_type;          // see ENUM list above 
    float period;                     // seconds for a single period
    float duty_cycle;                 // percent of period that interference 
                                      // is ON.  expressed as a float 
                                      // between 0.0 and 1.0

    // interferer freq hop parameters
    int   tx_freq_hop_type;            // NONE | ALTERNATING | SWEEP | RANDOM
    float tx_freq_hop_min;             // center frequency minimum
    float tx_freq_hop_max;             // center frequency maximum
    float tx_freq_hop_dwell_time;      // seconds at a given freq
    float tx_freq_hop_increment;       // for SWEEP, increment hop amount 
    
    // gmsk interferer properties 
    unsigned int gmsk_header_length;
    unsigned int gmsk_payload_length; 
    float        gmsk_bandwidth;       // note tx_rate is ignored and 
                                       // calcualted from gmsk_bandwidth 
                                       // (default = 4.0f * bandwidth)

};

#endif
