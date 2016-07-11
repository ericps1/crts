#ifndef _SC_BER_SWEEP_
#define _SC_BER_SWEEP_

#include "scenario_controller.hpp"
#include "timer.h"

#define BER_SWEEP_LOG_FILE ("./BER_SWEEP.m")

class SC_BER_Sweep : public ScenarioController {

private:
  // internal members used by this CE
  double tx_gain;
  double rx_gain;
  int feedback_count;
  FILE *log;
  struct ExtensibleCognitiveRadio::rx_statistics rx_stats;
  int data_ind;

  static double constexpr tx_gain_min = 5.0;
  static double constexpr tx_gain_max = 20.0;
  static double constexpr tx_gain_step = 1.0;
  static double constexpr rx_gain_min = 5.0;
  static double constexpr rx_gain_max = 20.0;
  static double constexpr rx_gain_step = 1.0;
  static double constexpr dwell_time_s = 120.0;
  static double constexpr settle_time_us = 1e6;
  
public:
  SC_BER_Sweep(int argc, char **argv);
  ~SC_BER_Sweep();
  virtual void execute();
  virtual void initialize_node_fb();
};

#endif
