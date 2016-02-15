#include "SC.hpp"

Scenario_Controller::Scenario_Controller() {}
Scenario_Controller::~Scenario_Controller() {}
void Scenario_Controller::execute(int node_number, char fb_type, void * _arg) {}

void Scenario_Controller::initialize_node_fb() {}

void Scenario_Controller::set_node_parameters(int node, char cont_type, void* _arg){
  char cont_msg[16];
  int cont_msg_len;
  
  cont_msg[0] = CRTS_MSG_CONTROL;
  cont_msg[2] = cont_type;
  switch(cont_type){
    case CRTS_TX_STATE:
    case CRTS_TX_MOD:
    case CRTS_TX_FEC0:
    case CRTS_TX_FEC1:
    case CRTS_RX_STATE:
    case CRTS_NET_MODEL:
    case CRTS_FB_EN:
      memcpy((void*)&cont_msg[3], _arg, sizeof(int));
      cont_msg_len = 3+sizeof(int);
      break;
    case CRTS_TX_FREQ:
    case CRTS_TX_RATE:
    case CRTS_TX_GAIN:
    case CRTS_RX_FREQ:
    case CRTS_RX_RATE:
    case CRTS_RX_GAIN:
    case CRTS_RX_STATS:
    case CRTS_RX_STATS_FB:
    case CRTS_NET_THROUGHPUT:
      memcpy((void*)&cont_msg[3], _arg, sizeof(double));
      cont_msg_len = 3+sizeof(double);
      break;
    default:
      printf("set_node_parameters() was passed an unknown control message type\n");
      exit(1);
      break;
  }

  // this field indicates the length of the control content i.e. the control type + value
  cont_msg[1] = cont_msg_len - 2;

  if (node > num_nodes) {
    printf("set_node_parameters() was called for a node which exceeds the number of nodes in this scenario\n");
    exit(1);
  } else {
    write(TCP_nodes[node-1], cont_msg, cont_msg_len);
  }
}
