#ifndef _CE_NETWORK_LOADING_
#define _CE_NETWORK_LOADING_

#include "extensible_cognitive_radio.hpp"
#include "cognitive_engine.hpp"
#include "timer.h"

class CE_Network_Loading : public CognitiveEngine {

private:
  bool net_saturated;
  timer t_settle;
  static float constexpr settle_period = 2.0;     // minimum time before between 
                                                  // consecutive requests
  static int constexpr sat_thresh_upper = 300000; // threshold for network saturation
  static int constexpr sat_thresh_lower = 100000; // threshold for network de-saturation
  unsigned char bw_cfg;          // indicates current bandwidth (subcarrier allocation)
  unsigned char tx_bw_cfg_req;   // this nodes bandwidth request
  unsigned char rx_bw_cfg_req;   // bandwidth request received from other node
  bool active_bw_cfg_req;        // flag for current bandwidth request

  unsigned char tx_control_info[ECR_CONTROL_INFO_BYTES];
  unsigned char rx_control_info[ECR_CONTROL_INFO_BYTES];

  void set_bw_cfg(ExtensibleCognitiveRadio* ECR, int bw_cfg);

  // subcarrier allocations
  char sc_alloc_0[256];
  char sc_alloc_1[256];
  char sc_alloc_2[256];

  // number of guard subcarriers (effectively setting bandwidth)
  static int constexpr guard_sc_0 = 104;
  static int constexpr guard_sc_1 = 80;
  static int constexpr guard_sc_2 = 32;

  static int constexpr num_sc = 256;      // total number of subcarriers
  static int constexpr sc_pilot_freq = 8; // frequency of pilot subcarriers
public:
  CE_Network_Loading(int argc, char **argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_Network_Loading();
  virtual void execute();
};

#endif
