#ifndef _INTERFERER_HPP_
#define _INTERFERER_HPP_

#include <uhd/usrp/multi_usrp.hpp>
#include <liquid/liquid.h>
#include "timer.h"

#define DEFAULT_RUN_TIME 20
#define DEFAULT_CONTROLLER_IP_ADDRESS "192.168.1.56"

#define USRP_BUFFER_LENGTH 256
#define TX_BUFFER_LENGTH 5120

#define GMSK_PAYLOAD_LENGTH 50
#define GMSK_HEADER_LENGTH 8

#define RRC_SAMPS_PER_SYM 2
#define RRC_FILTER_SEMILENGTH 32
#define RRC_BETA 0.35

#define OFDM_CP_LENGTH 4
#define OFDM_TAPER_LENGTH 4
#define OFDM_HEADER_LENGTH 8
#define OFDM_PAYLOAD_LENGTH 128

#define DAC_RATE 64e6

class Interferer {
public:
  Interferer(/*string with name of CE_execute function*/);
  ~Interferer();

  // Interference parameters
  int interference_type;
  float tx_gain_soft; // soft transmit gain (linear)
  float tx_gain;
  float tx_freq;
  float tx_rate;

  float period;
  float duty_cycle;

  int tx_freq_hop_type;
  float tx_freq_hop_min;
  float tx_freq_hop_max;
  float tx_freq_hop_dwell_time;
  float tx_freq_hop_increment;

  // RF objects and properties
  uhd::usrp::multi_usrp::sptr usrp_tx;
  uhd::tx_metadata_t metadata_tx;

private:
};

#endif
