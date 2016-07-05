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
  set_node_parameter(1, CRTS_FB_EN, (void*) &fb_enables);
  set_node_parameter(2, CRTS_FB_EN, (void*) &fb_enables);

  double rx_stats_period = 10.0;
  set_node_parameter(1, CRTS_RX_STATS, (void*) &rx_stats_period);
  set_node_parameter(2, CRTS_RX_STATS, (void*) &rx_stats_period);
  
  set_node_parameter(1, CRTS_RX_STATS_FB, (void*) &rx_stats_period);
  set_node_parameter(2, CRTS_RX_STATS_FB, (void*) &rx_stats_period);
}

// execute function
void SC_Control_and_Feedback_Test::execute() {

  if (sc_event == FEEDBACK) {
    
    // handle all possible feedback types
    switch (fb.fb_type) {
      case CRTS_TX_STATE:
        if(np[fb.node-1].node_type == COGNITIVE_RADIO) {
          if (*(int*)fb.arg == TX_STOPPED)
            printf("Node %i has stopped transmitting\n\n", fb.node);
          if (*(int*)fb.arg == TX_CONTINUOUS)
            printf("Node %i has started transmitting\n\n", fb.node);
        } else {
          if (*(int*)fb.arg == INT_TX_STOPPED)
            printf("Node %i has stopped transmitting\n\n", fb.node);
          if (*(int*)fb.arg == INT_TX_DUTY_CYCLE_ON)
            printf("Node %i has started the on period of it's duty cycle\n\n", fb.node);
          if (*(int*)fb.arg == INT_TX_DUTY_CYCLE_OFF)
            printf("Node %i has started the off period of it's duty cycle\n\n", fb.node);
        }
        break;
      case CRTS_TX_FREQ:
        printf("Node %i has updated it's transmit frequency to %.3e\n\n", fb.node, *(double*)fb.arg);
        break;
      case CRTS_TX_RATE:
        printf("Node %i has updated it's transmit rate to %.3e\n\n", fb.node, *(double*)fb.arg);
        break;
      case CRTS_TX_GAIN:
        printf("Node %i has updated it's transmit gain to %.3f\n\n", fb.node, *(double*)fb.arg);
        break;
      case CRTS_TX_MOD:
        printf("Node %i has updated it's transmit modulation to %s\n\n", fb.node, 
               modulation_types[*(int*)fb.arg].name);
        break;
      case CRTS_TX_CRC:
        printf("Node %i has updated it's transmit crc to %s\n", fb.node, 
                crc_scheme_str[*(int*)fb.arg][0]);
        break;
      case CRTS_TX_FEC0:
        printf("Node %i has updated it's inner FEC scheme  to %s\n\n", fb.node, 
               fec_scheme_str[*(int*)fb.arg][0]);
        break;
      case CRTS_TX_FEC1:
        printf("Node %i has updated it's outter FEC scheme to %s\n\n", fb.node, 
               fec_scheme_str[*(int*)fb.arg][0]);
        break; 
      case CRTS_RX_STATE:
        if (*(int*)fb.arg == RX_STOPPED)
          printf("Node %i has stopped receiving\n\n", fb.node);
        if (*(int*)fb.arg == RX_CONTINUOUS)
          printf("Node %i has started receiving\n\n", fb.node);
        break;
      case CRTS_RX_FREQ:
        printf("Node %i has updated it's receive frequency to %.3e\n\n", fb.node, *(double*)fb.arg);
        break;
      case CRTS_RX_RATE:
        printf("Node %i has updated it's receive rate to %.3e\n\n", fb.node, *(double*)fb.arg);
        break;
      case CRTS_RX_GAIN:
        printf("Node %i has updated it's receive gain to %.1f\n\n", fb.node, *(double*)fb.arg);
        break;
      case CRTS_RX_STATS:
        struct ExtensibleCognitiveRadio::rx_statistics rx_stats = 
          *(struct ExtensibleCognitiveRadio::rx_statistics*) fb.arg;
        printf("Node %i has sent updated receive statistics:\n", fb.node);
        printf("  Number of frames received: %i\n", rx_stats.frames_received);
        printf("  Average BER:               %.3e\n", rx_stats.ber);
        printf("  Average PER:               %.3f\n", rx_stats.per);
        printf("  Average throughput:        %.3e\n", rx_stats.throughput);
        printf("  Average EVM:               %.3f\n", rx_stats.evm_dB);
        printf("  Average RSSI:              %.3f\n\n", rx_stats.rssi_dB);
        break;
    }
  }
}



