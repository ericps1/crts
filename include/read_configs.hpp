#ifndef _H_READ_CONFIG_
#define _H_READ_CONFIG_

#include <string>
#include <vector>

struct scenario_parameters{
    int num_nodes;
    float run_time;
};

int read_scenario_master_file(char scenario_list[30][60], unsigned int scenario_reps[60]);

struct scenario_parameters read_scenario_parameters(char * scenario_file);

struct node_parameters read_node_parameters(int node, char * scenario_file);

void print_node_parameters(struct node_parameters *np);

#endif
