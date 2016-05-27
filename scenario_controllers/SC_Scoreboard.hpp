#ifndef _SC_Scoreboard_
#define _SC_Scoreboard_

#include "timer.h"
#include "SC.hpp"

#define Scoreboard_PORT 4446
#define Scoreboard_IP "192.168.1.103"

struct num_nodes_struct
{
    int num_nodes;
};

struct feedback_struct
{
    int type;
    int node;
    float frequency;
    float bandwidth;
};

struct node_struct
{
    float frequency;
    float bandwidth;
    char team_name[200];
    int role;
    int node;
};


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
