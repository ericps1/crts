#ifndef _SC_Scoreboard_
#define _SC_Scoreboard_

#include "scenario_controller.hpp"

#define Scoreboard_PORT 4446
#define Scoreboard_IP "192.168.1.103"

class SC_Scoreboard : public ScenarioController {

private:
  // internal members used by this CE
  int TCP_Scoreboard;
  
public:
  SC_Scoreboard(int argc, char ** argv);
  ~SC_Scoreboard();
  virtual void execute();
  virtual void initialize_node_fb();
};

#endif
