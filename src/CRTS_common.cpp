#include "CRTS.hpp"
#include "ECR.hpp"

int get_control_arg_len(int control_type){
  
  int len;
  switch(control_type){
    case CRTS_TX_STATE:
    case CRTS_TX_MOD:
    case CRTS_TX_FEC0:
    case CRTS_TX_FEC1:
    case CRTS_RX_STATE:
    case CRTS_NET_MODEL:
    case CRTS_FB_EN:
    case CRTS_TX_FREQ_BEHAVIOR:
      len = sizeof(int);
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
    case CRTS_TX_DUTY_CYCLE:
    case CRTS_TX_PERIOD:
    case CRTS_TX_FREQ_MIN:
    case CRTS_TX_FREQ_MAX:
    case CRTS_TX_FREQ_DWELL_TIME:
    case CRTS_TX_FREQ_RES:
      len = sizeof(double);
      break;
    default:
      len = 0;
  }

  return len;
}

int get_feedback_arg_len(int fb_type){
  
  int len;
  switch(fb_type){
    case CRTS_TX_STATE:
    case CRTS_TX_MOD:
    case CRTS_TX_FEC0:
    case CRTS_TX_FEC1:
    case CRTS_RX_STATE:
      len = sizeof(int);
      break;
    case CRTS_TX_FREQ:
    case CRTS_TX_RATE:
    case CRTS_TX_GAIN:
    case CRTS_RX_FREQ:
    case CRTS_RX_RATE:
    case CRTS_RX_GAIN:
      len = sizeof(double);
      break;
    case CRTS_RX_STATS:
      len = sizeof(struct ExtensibleCognitiveRadio::rx_statistics);
      break;
    default:
      len = 0;
  }

  return len;
}
