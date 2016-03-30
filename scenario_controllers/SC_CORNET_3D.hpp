#ifndef _SC_CORNET_3D_
#define _SC_CORNET_3D_

#include "SC.hpp"

#define CORNET_3D_PORT 4444
#define CORNET_3D_IP "192.168.1.103"

class SC_CORNET_3D : public Scenario_Controller {

private:
  // internal members used by this CE
  int TCP_CORNET_3D;
  
  //store previous values so we don't make unnecessary updates to the radios
  int old_mod;
  int old_crc;
  int old_fec0;
  int old_fec1;
  double old_freq;
  double old_bandwidth;
  double old_gain;

public:
  SC_CORNET_3D();
  ~SC_CORNET_3D();
  virtual void execute(int node, char fb_type, void *_arg);
  virtual void initialize_node_fb();
};

#endif
