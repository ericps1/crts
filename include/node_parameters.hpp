#ifndef _H_NODE_PARAMS_
#define _H_NODE_PARAMS_

#include <string>
#include <liquid/liquid.h>

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

enum duplex {
  FDD = 0, // frequency division duplexing
  TDD,     // time division duplexing (not implemented)
  HD       // half-duplex
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
  
  // log/print settings
  int print_metrics;
  int log_phy_rx;
  int log_phy_tx;
  int log_net_rx;
  int log_net_tx;
  char phy_rx_log_file[100];
  char phy_tx_log_file[100];
  char net_rx_log_file[100];
  char net_tx_log_file[100];
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
  int duplex;
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

  // A getopt style string that can contain custom parameters to be passed to the CE.
  char custom_param_str[2048]; 
};

#endif
