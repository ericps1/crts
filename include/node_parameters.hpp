#ifndef _H_NODE_PARAMS_
#define _H_NODE_PARAMS_

#include<string>

enum type{
    CR = 0,				// user equipment node type
    interferer		// interferer node type
};

enum duplex{
	FDD = 0,		// frequency division duplexing
	TDD,			// time division duplexing (not implemented)
	HD				// half-duplex
};

enum traffic{
    stream = 0,
    burst
};

enum int_type{
    CW = 0,			// continuous-wave interference
    AWGN,			// additive white gaussian noise interference
	GMSK,			// gaussian minimum-shift keying inteference
	RRC,			// root-raised cosine interference (as in WCDMA)
	OFDM			// orthogonal frequency division multiplexing interference
};

struct node_parameters{
	// general
    int type;
    char CORNET_IP[20];
    char CRTS_IP[20];
    char TARGET_IP[20];
	char CE[30];
    int layers;
    int traffic;
	int print_metrics;
	int log_metrics;
	char log_file[30];
    float ce_timeout_ms;

	// RF
	int duplex;
    float tx_freq;
    float tx_rate;
    float tx_gain_soft;
    float tx_gain;
    float tx_delay_us;
	float rx_freq;
    float rx_rate;
    float rx_gain;
    
    // interferer only
    int int_type;
    float period;
	float duty_cycle;
};

#endif
