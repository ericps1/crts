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

    int type;
    std::string CORNET_IP;
    std::string CRTS_IP;
    std::string CE;
    int layers;
    int traffic;

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
    int signal;
    float duty_cycle;
};

#endif
