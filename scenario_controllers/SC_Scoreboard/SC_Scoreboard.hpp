#ifndef _SC_Scoreboard_
#define _SC_Scoreboard_

#include "timer.h"
#include "scenario_controller.hpp"
#include "CORNET_3D.hpp"

#define Scoreboard_PORT 4446
#define Scoreboard_IP "192.168.1.103"

class SC_Scoreboard : public ScenarioController {

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
  
  int state;
  timer t;
  static constexpr float period = 15.0;
  double throughput_min = 1e4;
  double throughput_max = 1e6;
  //std::mt19937 randomGenerator;
  //std::uniform_real_distribution<double> getRandomThroughput;
  bool first_execution;

public:
  SC_Scoreboard(int argc, char** argv);
  ~SC_Scoreboard();
  virtual void execute();
  virtual void initialize_node_fb();
  double throughput1;
  double throughput2;
  double throughput3;
  double throughput4;
};

#endif
