#include "SC.hpp"

Scenario_Controller::Scenario_Controller() {}
Scenario_Controller::~Scenario_Controller() {}
void Scenario_Controller::execute(int node_number, char fb_type, void * _arg) {}

void Scenario_Controller::initialize_node_fb() {}

void Scenario_Controller::set_node_parameter(int node, char cont_type, void* _arg){
  char cont_msg[16];
  int arg_len = get_control_arg_len(cont_type);

  cont_msg[0] = CRTS_MSG_CONTROL;
  cont_msg[1] = cont_type;
  if(arg_len > 0)
    memcpy((void*)&cont_msg[2], _arg, arg_len);
        
  if (node > sp.num_nodes) {
    printf("set_node_parameters() was called for a node which exceeds the number of nodes in this scenario\n");
    exit(1);
  } else {
    write(TCP_nodes[node], cont_msg, 2+arg_len);
  }
}
