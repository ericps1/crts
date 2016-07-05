#ifndef _SC_CORNET_Display_
#define _SC_CORNET_Display_

#include "scenario_controller.hpp"
#include "CORNET_3D.hpp"

#define CORNET_Display_PORT 4446
#define CORNET_Display_IP "192.168.1.103"

class SC_CORNET_Display : public ScenarioController {

private:
  // internal members used by this CE
  int TCP_CORNET_Display;
  double *old_frequencies;
  double* old_bandwidths;

public:
  SC_CORNET_Display(int argc, char **argv);
  ~SC_CORNET_Display();
  virtual void execute();
  virtual void initialize_node_fb();
};

#endif
