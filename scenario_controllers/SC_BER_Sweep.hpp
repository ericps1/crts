#ifndef _SC_BER_SWEEP_
#define _SC_BER_SWEEP_

#include "SC.hpp"
#include "timer.h"

#define BER_SWEEP_TX_GAIN_MIN (5.0)
#define BER_SWEEP_TX_GAIN_MAX (20.0)
#define BER_SWEEP_TX_GAIN_STEP (0.1)

#define BER_SWEEP_RX_GAIN_MIN (5.0)
#define BER_SWEEP_RX_GAIN_MAX (20.0)
#define BER_SWEEP_RX_GAIN_STEP (0.1)

#define BER_SWEEP_DWELL_TIME_S (120.0)
#define BER_SWEEP_SETTLE_TIME_US (1e6)

#define BER_SWEEP_LOG_FILE ("./BER_SWEEP.m")

class SC_BER_Sweep : public Scenario_Controller {

private:
  // internal members used by this CE
  double tx_gain;
  double rx_gain;
  int feedback_count;
  FILE *log;
  struct ExtensibleCognitiveRadio::rx_statistics rx_stats;
  int data_ind;
public:
  SC_BER_Sweep(int argc, char **argv);
  ~SC_BER_Sweep();
  virtual void execute(int node, char fb_type, void *_arg);
  virtual void initialize_node_fb();
};

#endif
