#ifndef _SC_CORNET_Display_
#define _SC_CORNET_Display_

#include "scenario_controller.hpp"
#include "CORNET_3D.hpp"

#define CORNET_Display_PORT 4444
#define CORNET_Display_IP "192.168.1.103"

class SC_CORNET_Display : public ScenarioController {

private:
  // internal members used by this CE
  
  // Socket connection to cornet_3d backend
  int TCP_CORNET_Display;

  // Arrays to store signal data for scoreboard functionality
  double *old_frequencies;
  double* old_bandwidths;
  
  // Caches of previous values so the scenario controller only updates
  // parameters when they change
  int old_mod;
  int old_crc;
  int old_fec0;
  int old_fec1;
  double old_freq;
  double old_bandwidth;
  double old_gain;

public:
  SC_CORNET_Display(int argc, char **argv);
  ~SC_CORNET_Display();
  virtual void execute();
  virtual void initialize_node_fb();
};

#endif
