#ifndef _H_NODE_PARAMS_
#define _H_NODE_PARAMS_

#include<string>

enum type{
    BS,
    UE,
    interferer
};

enum traffic{
    stream,
    burst
};

enum int_type{
    CW,
    RRC
};

struct node_parameters{
	// general
    int type;
    char CORNET_IP[16];
    char CRTS_IP[16];
    char CE[30];
    int layers;
    int traffic;
	int print_metrics;
	int log_metrics;
	char log_file[30];

	// RF
    float freq_tx;
    float freq_rx;
    float tx_rate;
    float rx_rate;
    float gain_tx_soft;
    float gain_tx;
    float gain_rx;
    float max_gain_tx;
    float max_gain_rx;

    // interferer only
    int int_type;
    float duty_cycle;
};

#endif
