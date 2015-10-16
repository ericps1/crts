#ifndef _INTERFERER_HPP_
#define _INTERFERER_HPP_

#include <uhd/usrp/multi_usrp.hpp>
#include <liquid/liquid.h>
#include "timer.h"

class Interferer {
public:
    Interferer(/*string with name of CE_execute function*/);
    ~Interferer();

    // Interference parameters
    int interference_type;
    float tx_gain_soft;                  // soft transmit gain (linear)
    float tx_gain;
    float tx_freq;
    float tx_rate;

    float period;
    float duty_cycle;

    int   tx_freq_hop_type; 
    float tx_freq_hop_min;
    float tx_freq_hop_max;
    float tx_freq_hop_dwell_time; 
    float tx_freq_hop_increment; 
    //timer onTimer;
	//timer dwellTimer;

    unsigned int gmsk_header_length;
    unsigned int gmsk_payload_length;
    float gmsk_bandwidth; 


    unsigned int gmskHeaderLength;
	unsigned int gmskPayloadLength;

    // RF objects and properties
    uhd::usrp::multi_usrp::sptr usrp_tx;
    uhd::tx_metadata_t          metadata_tx;

    //gmskframegen *gmsk_fg;
	//resamp2_crcf *interp;

private:
};

#endif
