#ifndef _CRTS_HPP_
#define _CRTS_HPP_

#define CRTS_TCP_CONTROL_PORT 4444
#define CRTS_CR_PORT 4444
#define CRTS_CR_NET_PACKET_LEN 256

enum crts_msg_type {
  CRTS_MSG_SCENARIO_PARAMETERS = 0,
  CRTS_MSG_MANUAL_START,
  CRTS_MSG_TERMINATE,
  CRTS_MSG_CONTROL,
  CRTS_MSG_FEEDBACK
};

// enumeration of all types of control and feedback passed between 
// the controller and all other nodes during an experiment
enum crts_ctrl_and_fdbk_type {
  CRTS_TX_STATE = 0,
  CRTS_TX_FREQ,
  CRTS_TX_RATE,
  CRTS_TX_GAIN,
  CRTS_TX_MOD,
  CRTS_TX_FEC0,
  CRTS_TX_FEC1,

  CRTS_RX_STATE,
  CRTS_RX_FREQ,
  CRTS_RX_RATE,
  CRTS_RX_GAIN,
  CRTS_RX_STATS,
  CRTS_RX_STATS_FB,

  CRTS_NET_THROUGHPUT,
  CRTS_NET_MODEL,

  CRTS_FB_EN
};

// defines bit masks used for feedback enables
#define CRTS_TX_STATE_FB_EN       (1<<CRTS_TX_STATE)
#define CRTS_TX_FREQ_FB_EN        (1<<CRTS_TX_FREQ)
#define CRTS_TX_RATE_FB_EN        (1<<CRTS_TX_RATE)
#define CRTS_TX_GAIN_FB_EN        (1<<CRTS_TX_GAIN)
#define CRTS_TX_MOD_FB_EN         (1<<CRTS_TX_MOD)
#define CRTS_TX_FEC0_FB_EN        (1<<CRTS_TX_FEC0)
#define CRTS_TX_FEC1_FB_EN        (1<<CRTS_TX_FEC1)

#define CRTS_RX_STATE_FB_EN       (1<<CRTS_RX_STATE)
#define CRTS_RX_FREQ_FB_EN        (1<<CRTS_RX_FREQ)
#define CRTS_RX_RATE_FB_EN        (1<<CRTS_RX_RATE)
#define CRTS_RX_GAIN_FB_EN        (1<<CRTS_RX_GAIN)
#define CRTS_RX_STATS_FB_EN       (1<<CRTS_RX_STATS)

void set_node_parameter(int node, char cont_type, void* _arg);

int get_control_arg_len(int control_type);
int get_feedback_arg_len(int fb_type);

#endif
