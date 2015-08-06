#ifndef _INTERFERER_HPP_
#define _INTERFERER_HPP_

#include <uhd/usrp/multi_usrp.hpp>

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
	float duty_cycle;
        float dwell_time; 
	float period_duration;
        int   tx_freq_hop_type; 
        float tx_freq_min;
        float tx_freq_max;
        float gmsk_header_length;
        float gmsk_payload_length;
        float gmsk_bandwidth; 


    // RF objects and properties
    uhd::usrp::multi_usrp::sptr usrp_tx;
    uhd::tx_metadata_t          metadata_tx;
	
private:
};

#endif
