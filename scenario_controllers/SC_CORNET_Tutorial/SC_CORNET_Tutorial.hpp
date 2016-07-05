#ifndef _SC_CORNET_Tutorial_
#define _SC_CORNET_Tutorial_

#include "scenario_controller.hpp"
#include "CORNET_3D.hpp"

#define CORNET_Tutorial_PORT 4444
#define CORNET_Tutorial_IP "192.168.1.103"

class SC_CORNET_Tutorial : public ScenarioController {

private:
  // internal members used by this CE
  int TCP_CORNET_Tutorial;
  
  //store previous values so we don't make unnecessary updates to the radios
  int old_mod;
  int old_crc;
  int old_fec0;
  int old_fec1;
  double old_freq;
  double old_bandwidth;
  double old_gain;

public:
  SC_CORNET_Tutorial(int argc, char **argv);
  ~SC_CORNET_Tutorial();
  virtual void execute();
  virtual void initialize_node_fb();
};

#endif
