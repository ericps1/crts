#ifndef _H_READ_CONFIG_
#define _H_READ_CONFIG_

#include <string>
#include <vector>

struct scenario_parameters {

  // Number of nodes in the scenario
  int num_nodes;

  // The start time of the scenario
  time_t start_time_s;

  // The length of time to run the scenario
  time_t runTime;

  // Total number of times this scenario
  // will be run
  unsigned int totalNumReps;

  // The repetition number of this scenario instance
  // i.e. 1 <= repNumber <= totalNumReps
  unsigned int repNumber;
};

enum msg_type { 
  CRTS_MSG_SCENARIO_PARAMETERS = 0, 
  CRTS_MSG_MANUAL_START, 
  CRTS_MSG_TERMINATE
};

int read_master_num_scenarios();

int read_master_scenario(int scenario_num,
                              char * scenario_name);

struct scenario_parameters read_scenario_parameters(char *scenario_file);

struct node_parameters read_node_parameters(int node, char *scenario_file);

void print_node_parameters(struct node_parameters *np);

#endif
