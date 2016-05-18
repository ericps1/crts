#ifndef _CRTS_HPP_
#define _CRTS_HPP_

#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <liquid/liquid.h>
#include <cstdint>

/////////////////////////////////////////////////////////////////
// Command-line style arguments
/////////////////////////////////////////////////////////////////

void str2argcargv(char *string, char *progName, int &argc, char (**&argv));

void freeargcargv(int &argc, char **&argv);

/////////////////////////////////////////////////////////////////
// Config files
/////////////////////////////////////////////////////////////////

int read_master_num_scenarios(char * nameMasterScenFile);

int read_master_scenario(char * nameMasterScenFile, int scenario_num,
                              char * scenario_name);

struct scenario_parameters read_scenario_parameters(char *scenario_file);

struct node_parameters read_node_parameters(int node, char *scenario_file);

//////////////////////////////////////////////////////////////////
// Scenario parameters
//////////////////////////////////////////////////////////////////

struct scenario_parameters {

  // Number of nodes in the scenario
  int num_nodes;

  // The start time of the scenario
  //time_t start_time_s;
  int64_t start_time_s;

  // The length of time to run the scenario
  //time_t runTime;
  int64_t runTime;

  // Total number of times this scenario
  // will be run
  unsigned int totalNumReps;

  // The repetition number of this scenario instance
  // i.e. 1 <= repNumber <= totalNumReps
  unsigned int repNumber;

  // Scenario controller
  char SC[100];
  float sc_timeout_ms;
  char sc_args[100];
};

//////////////////////////////////////////////////////////////////
// Node parameters
//////////////////////////////////////////////////////////////////

enum node_type {
  CR = 0,    // cognitive radio node type
  interferer // interferer node type
};

enum cr_type {
  python = 0, // third party python radios
  ecr // Radios created using ECR
};

enum net_traffic_type {
  NET_TRAFFIC_STREAM = 0,
  NET_TRAFFIC_BURST,
  NET_TRAFFIC_POISSON
};

enum interference_type {
  CW = 0, // continuous-wave interference
  NOISE,  // random noise interference
  GMSK,   // gaussian minimum-shift keying inteference
  RRC,    // root-raised cosine interference (as in WCDMA)
  OFDM    // orthogonal frequency division multiplexing interference
};

enum tx_freq_behavior { FIXED = 0, SWEEP, RANDOM };

enum subcarrier_alloc {
  LIQUID_DEFAULT_SUBCARRIER_ALLOC = 0,
  STANDARD_SUBCARRIER_ALLOC,
  CUSTOM_SUBCARRIER_ALLOC
};

struct node_parameters {
  // general
  int type;
  int cr_type;
  char python_file[100];
  char arguments[20][50];
  int num_arguments;
  
  // network settings
  char CORNET_IP[20];
  char CRTS_IP[20];
  char TARGET_IP[20];
  int net_traffic_type;
  int net_burst_length;
  double net_mean_throughput;
  
  
  // CE settings
  char CE[100];
  double ce_timeout_ms;
  char ce_args[2048]; 
  
  // log/print settings
  int print_metrics;
  int log_phy_rx;
  int log_phy_tx;
  int log_net_rx;
  int log_net_tx;
  char phy_rx_log_file[260];
  char phy_tx_log_file[260];
  char net_rx_log_file[260];
  char net_tx_log_file[260];
  int generate_octave_logs;
  int generate_python_logs;

  // USRP settings
  double rx_freq;
  double rx_rate;
  double rx_gain;
  double tx_freq;
  double tx_rate;
  double tx_gain;

  // liquid OFDM settings
  int rx_subcarriers;
  int rx_cp_len;
  int rx_taper_len;
  int rx_subcarrier_alloc_method;
  int rx_guard_subcarriers;
  int rx_central_nulls;
  int rx_pilot_freq;
  char rx_subcarrier_alloc[2048];

  double tx_gain_soft;
  int tx_subcarriers;
  int tx_cp_len;
  int tx_taper_len;
  int tx_modulation;
  int tx_crc;
  int tx_fec0;
  int tx_fec1;
  int tx_subcarrier_alloc_method;
  int tx_guard_subcarriers;
  int tx_central_nulls;
  int tx_pilot_freq;
  char tx_subcarrier_alloc[2048];

  // interferer only
  int interference_type; // see ENUM list above
  double period;          // seconds for a single period
  double duty_cycle;      // percent of period that interference
                         // is ON.  expressed as a float
                         // between 0.0 and 1.0

  // interferer freq hop parameters
  int tx_freq_behavior;     // NONE | ALTERNATING | SWEEP | RANDOM
  double tx_freq_min;        // center frequency minimum
  double tx_freq_max;        // center frequency maximum
  double tx_freq_dwell_time; // seconds at a given freq
  double tx_freq_resolution; // granularity for SWEEP and RANDOM frequency behaviors

};

void print_node_parameters(struct node_parameters *np);

//////////////////////////////////////////////////////////////////
// Control and feedback
//////////////////////////////////////////////////////////////////

#define CRTS_TCP_CONTROL_PORT 4444
#define CRTS_CR_PORT 4444
#define CRTS_CR_PACKET_LEN 256        // length of network packet data
#define CRTS_CR_PACKET_NUM_LEN 4      // number of bytes used for packet numbering
#define CRTS_CR_PACKET_SR_LEN 12      // shift register length for pseudo-random packet generation

enum crts_msg_type {
  CRTS_MSG_SCENARIO_PARAMETERS = 0,
  CRTS_MSG_START,
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
  CRTS_RX_STATS_RESET,

  CRTS_NET_THROUGHPUT,
  CRTS_NET_MODEL,

  CRTS_FB_EN,
  
  // interferer specific parameters
  CRTS_TX_DUTY_CYCLE,
  CRTS_TX_PERIOD,
  CRTS_TX_FREQ_BEHAVIOR,
  CRTS_TX_FREQ_MIN,
  CRTS_TX_FREQ_MAX,
  CRTS_TX_FREQ_DWELL_TIME,
  CRTS_TX_FREQ_RES
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
