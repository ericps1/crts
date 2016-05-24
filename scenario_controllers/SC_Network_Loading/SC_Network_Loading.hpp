#ifndef _SC_NETWORK_LOADING_
#define _SC_NETWORK_LOADING_

#include "scenario_controller.hpp"
#include "timer.h"

class SC_Network_Loading : public ScenarioController {

private:
  // internal members used by this CE
  int state;
  timer t;
  static constexpr float period = 15.0;
  double throughput_1;
  double throughput_2;

public:
  SC_Network_Loading(int argc, char **argv);
  ~SC_Network_Loading();
  virtual void execute();
  virtual void initialize_node_fb();
};

#endif
