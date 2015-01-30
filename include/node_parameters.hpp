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
    char CORNET_IP[20];
    char CRTS_IP[20];
    char CE[30];
    int layers;
    int traffic;
	int print_metrics;
	int log_metrics;
	char log_file[30];

	// RF
    float tx_freq;
    float rx_freq;
    float tx_rate;
    float rx_rate;
    float tx_gain_soft;
    float tx_gain;
    float rx_gain;
    float tx_max_gain;
    float rx_max_gain;

    // interferer only
    int int_type;
    float period;
	float duty_cycle;
};

#endif
