#ifndef _H_READ_CONFIG_
#define _H_READ_CONFIG_

int readScMasterFile(char scenario_list[30][60], int verbose);

int read_num_nodes(char *scenario_file);

struct node_parameters read_node_parameters(int node, char *scenario_file);

void print_node_parameters(struct node_parameters *np);

#endif
