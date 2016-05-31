#ifndef _SC_Scoreboard_
#define _SC_Scoreboard_

#include "timer.h"
#include "SC.hpp"
#include "CORNET_3D.hpp"

#define Scoreboard_PORT 4446
#define Scoreboard_IP "192.168.1.103"

class SC_Scoreboard : public Scenario_Controller {

private:
  // internal members used by this CE
  int TCP_Scoreboard;
  double* old_frequencies;
  double* old_bandwidths;
  double test_frequency0 = 766e6;
  double test_frequency1 = 768e6;
  double test_bandwidth = 1e6;
  int flip = 1;
  timer switch_timer;
  timer* throughput_timers;
public:
  SC_Scoreboard();
  ~SC_Scoreboard();
  virtual void execute();
  virtual void initialize_node_fb();
};

#endif
