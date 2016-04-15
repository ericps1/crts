#include <stdio.h>
#include <liquid/liquid.h>
#include "SC_Template.hpp"

// constructor
SC_Template::SC_Template() {}

// destructor
SC_Template::~SC_Template() {}

// setup feedback enables for each fb.node
void SC_Template::initialize_node_fb() {
  // initialize node feedback settings
  int fb_enables = INT_MAX; //CRTS_RX_STATS_FB_EN; 
  set_node_parameter(0, CRTS_FB_EN, (void*) &fb_enables);
  set_node_parameter(1, CRTS_FB_EN, (void*) &fb_enables);
  
  double rx_stats_tracking_period = 10.0;
  set_node_parameter(0, CRTS_RX_STATS, (void*) &rx_stats_tracking_period);
  set_node_parameter(1, CRTS_RX_STATS, (void*) &rx_stats_tracking_period);
  
  double rx_stats_fb_period = 10.0;
  set_node_parameter(0, CRTS_RX_STATS_FB, (void*) &rx_stats_fb_period);
  set_node_parameter(1, CRTS_RX_STATS_FB, (void*) &rx_stats_fb_period);
}

// execute function
void SC_Template::execute() {

  if (sc_event == TIMEOUT) {
    //printf("Scenario Controller was triggered by a timeout\n");
  }

  if (sc_event == FEEDBACK) {
    // handle all possible feedback types
    switch (fb.fb_type) {
      case CRTS_TX_STATE:
        if (*(int*)fb.arg == TX_STOPPED)
          printf("Node %i has stopped transmitting\n", fb.node);
        if (*(int*)fb.arg == TX_CONTINUOUS)
          printf("Node %i has started transmitting\n", fb.node);
        break;
      case CRTS_TX_FREQ:
        printf("Node %i has updated it's transmit frequency to %.1e\n", fb.node, *(double*)fb.arg);
        break;
      case CRTS_TX_RATE:
        printf("Node %i has updated it's transmit rate to %.3e\n", fb.node, *(double*)fb.arg);
        break;
      case CRTS_TX_GAIN:
        printf("Node %i has updated it's transmit gain to %.3f\n", fb.node, *(double*)fb.arg);
        break;
      case CRTS_TX_MOD:
        printf("Node %i has updated it's transmit modulation to %s\n", fb.node, 
               modulation_types[*(int*)fb.arg].name);
        break;
      case CRTS_TX_FEC0:
        printf("Node %i has updated it's inner FEC scheme  to %s\n", fb.node, 
               fec_scheme_str[*(int*)fb.arg][0]);
        break;
      case CRTS_TX_FEC1:
        printf("Node %i has updated it's outter FEC scheme to %s\n", fb.node, 
               fec_scheme_str[*(int*)fb.arg][0]);
        break;
    
      case CRTS_RX_STATE:
        if (*(int*)fb.arg == RX_STOPPED)
          printf("Node %i has stopped receiving\n", fb.node);
        if (*(int*)fb.arg == RX_CONTINUOUS)
          printf("Node %i has started receiving\n", fb.node);
        break;
      case CRTS_RX_FREQ:
        printf("Node %i has updated it's receive frequency to %.3e\n", fb.node, *(double*)fb.arg);
        break;
      case CRTS_RX_RATE:
        printf("Node %i has updated it's receive rate to %.3e\n", fb.node, *(double*)fb.arg);
        break;
      case CRTS_RX_GAIN:
        printf("Node %i has updated it's receive gain to %.1f\n", fb.node, *(double*)fb.arg);
        break;
      case CRTS_RX_STATS:
        struct ExtensibleCognitiveRadio::rx_statistics rx_stats = 
          *(struct ExtensibleCognitiveRadio::rx_statistics*) fb.arg;
        printf("Node %i has sent updated receive statistics:\n", fb.node);
        printf("  Number of frames received: %i\n", rx_stats.frames_received);
        printf("  Average EVM:               %.3f\n", rx_stats.avg_evm);
        printf("  Average RSSI:              %.3f\n", rx_stats.avg_rssi);
        printf("  Average PER:               %.3f\n", rx_stats.avg_per);
        printf("  Average throughput:        %.3e\n", rx_stats.avg_throughput);
        break;
     }
   }
}



