#ifndef _SC_Scoreboard_
#define _SC_Scoreboard_

#include "SC.hpp"

#define Scoreboard_PORT 4444
#define Scoreboard_IP "192.168.1.103"

class SC_Scoreboard : public Scenario_Controller {

private:
  // internal members used by this CE
  int TCP_Scoreboard;
  
  //store previous values so we don't make unnecessary updates to the radios
  int old_mod;
  int old_crc;
  int old_fec0;
  int old_fec1;
  double old_freq;
  double old_bandwidth;
  double old_gain;

public:
  SC_Scoreboard();
  ~SC_Scoreboard();
  virtual void execute();
  virtual void initialize_node_fb();
};

#endif
