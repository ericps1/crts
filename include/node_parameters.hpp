#ifndef _H_NODE_PARAMS_
#define _H_NODE_PARAMS_

#include<string>
#include<liquid/liquid.h>

enum type{
    BS = 0,			// basestation node type
    UE,				// user equipment node type
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
    char CE[30];
    int layers;
    int traffic;
	int print_metrics;
	int log_metrics;
	char log_file[30];
    float ce_timeout_length_ms;

	// RF
	int duplex;
    float tx_freq;
    float rx_freq;
    float tx_rate;
    float rx_rate;
    float tx_gain_soft;
    float tx_gain;
    float rx_gain;
    float tx_max_gain;
    float rx_max_gain;
    int tx_modulation;
    int tx_crc;
    int tx_fec0;
    int tx_fec1;

    // interferer only
    int int_type;
    float period;
	float duty_cycle;
};

#endif
