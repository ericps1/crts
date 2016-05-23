#include <stdio.h>
#include <liquid/liquid.h>
#include "SC_Control_and_Feedback_Test.hpp"

// constructor
SC_Control_and_Feedback_Test::SC_Control_and_Feedback_Test(int argc, char **argv) {}

// destructor
SC_Control_and_Feedback_Test::~SC_Control_and_Feedback_Test() {}

// setup feedback enables for each node
void SC_Control_and_Feedback_Test::initialize_node_fb() {
  printf("Sending control for fb enables\n");
  int fb_enables = INT_MAX; 
  set_node_parameter(0, CRTS_FB_EN, (void*) &fb_enables);
  set_node_parameter(1, CRTS_FB_EN, (void*) &fb_enables);
  set_node_parameter(2, CRTS_FB_EN, (void*) &fb_enables);

  double rx_stats_period = 1.0;
  set_node_parameter(0, CRTS_RX_STATS, (void*) &rx_stats_period);
  set_node_parameter(1, CRTS_RX_STATS, (void*) &rx_stats_period);
  
  set_node_parameter(0, CRTS_RX_STATS_FB, (void*) &rx_stats_period);
  set_node_parameter(1, CRTS_RX_STATS_FB, (void*) &rx_stats_period);
}

// execute function
void SC_Control_and_Feedback_Test::execute(int node, char fb_type, void *_arg) {

  printf("\n");
  
  // handle all possible feedback types
  switch (fb_type) {
    case CRTS_TX_STATE:
      if(np[node].node_type == COGNITIVE_RADIO) {
        if (*(int*)_arg == TX_STOPPED)
          printf("Node %i has stopped transmitting\n", node+1);
        if (*(int*)_arg == TX_CONTINUOUS)
          printf("Node %i has started transmitting\n", node+1);
      } else {
        if (*(int*)_arg == INT_TX_STOPPED)
          printf("Node %i has stopped transmitting\n", node+1);
        if (*(int*)_arg == INT_TX_DUTY_CYCLE_ON)
          printf("Node %i has started the on period of it's duty cycle\n", node+1);
        if (*(int*)_arg == INT_TX_DUTY_CYCLE_OFF)
          printf("Node %i has started the off period of it's duty cycle\n", node+1);
      }
      break;
    case CRTS_TX_FREQ:
      printf("Node %i has updated it's transmit frequency to %.3e\n", node+1, *(double*)_arg);
      break;
    case CRTS_TX_RATE:
      printf("Node %i has updated it's transmit rate to %.3e\n", node+1, *(double*)_arg);
      break;
    case CRTS_TX_GAIN:
      printf("Node %i has updated it's transmit gain to %.3f\n", node+1, *(double*)_arg);
      break;
    case CRTS_TX_MOD:
      printf("Node %i has updated it's transmit modulation to %s\n", node+1, 
             modulation_types[*(int*)_arg].name);
      break;
    case CRTS_TX_FEC0:
      printf("Node %i has updated it's inner FEC scheme  to %s\n", node+1, 
             fec_scheme_str[*(int*)_arg][0]);
      break;
    case CRTS_TX_FEC1:
      printf("Node %i has updated it's outter FEC scheme to %s\n", node+1, 
             fec_scheme_str[*(int*)_arg][0]);
      break; 
    case CRTS_RX_STATE:
      if (*(int*)_arg == RX_STOPPED)
        printf("Node %i has stopped receiving\n", node+1);
      if (*(int*)_arg == RX_CONTINUOUS)
        printf("Node %i has started receiving\n", node+1);
      break;
    case CRTS_RX_FREQ:
      printf("Node %i has updated it's receive frequency to %.3e\n", node+1, *(double*)_arg);
      break;
    case CRTS_RX_RATE:
      printf("Node %i has updated it's receive rate to %.3e\n", node+1, *(double*)_arg);
      break;
    case CRTS_RX_GAIN:
      printf("Node %i has updated it's receive gain to %.1f\n", node+1, *(double*)_arg);
      break;
    case CRTS_RX_STATS:
      struct ExtensibleCognitiveRadio::rx_statistics rx_stats = 
        *(struct ExtensibleCognitiveRadio::rx_statistics*) _arg;
      printf("Node %i has sent updated receive statistics:\n", node+1);
      printf("  Number of frames received: %i\n", rx_stats.frames_received);
      printf("  Average BER:               %.3e\n", rx_stats.ber);
      printf("  Average PER:               %.3f\n", rx_stats.per);
      printf("  Average throughput:        %.3e\n", rx_stats.throughput);
      printf("  Average EVM:               %.3f\n", rx_stats.evm_dB);
      printf("  Average RSSI:              %.3f\n", rx_stats.rssi_dB);
      break;
   }
}



