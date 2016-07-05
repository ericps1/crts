#include <stdio.h>
#include <liquid/liquid.h>
#include "SC_Performance_Sweep_Utility.hpp"

// constructor
SC_Performance_Sweep_Utility::SC_Performance_Sweep_Utility(int argc, char **argv) {

  // initialize parameters
  dwell_time_s = DEFAULT_DWELL_TIME_S;
  settle_time_s = DEFAULT_SETTLE_TIME_S;
  node_1_feedback_received = false;
  node_2_feedback_received = false;

  // set up sweep management
  sweep_mode = SWEEP_MODE_NESTED;
  data_ind = 0;
  num_sweep_params = 3;
  params.resize(num_sweep_params,0);
  num_vals.resize(num_sweep_params,0);
  vals.resize(num_sweep_params,0);
  indices.resize(num_sweep_params,0);
  rx_stats.resize(2);
  
  // define sweeping parameters and values
  params[0] = CRTS_TX_RATE;
  num_vals[0] = 3;
  vals[0] = (void*) new std::vector<double>;
  (*(std::vector<double>*)vals[0]).resize(num_vals[0]);
  (*(std::vector<double>*)vals[0])[0] = 1e6;
  (*(std::vector<double>*)vals[0])[1] = 2e6;
  (*(std::vector<double>*)vals[0])[2] = 5e6;

  params[1] = CRTS_TX_FEC0;
  num_vals[1] = 3;
  vals[1] = (void*) new std::vector<int>;
  (*(std::vector<int>*)vals[1]).resize(num_vals[1]);
  (*(std::vector<int>*)vals[1])[0] = LIQUID_FEC_NONE;
  (*(std::vector<int>*)vals[1])[1] = LIQUID_FEC_CONV_V27;
  (*(std::vector<int>*)vals[1])[2] = LIQUID_FEC_CONV_V29;

  params[2] = CRTS_TX_MOD;
  num_vals[2] = 4;
  vals[2] = (void*) new std::vector<int>;
  (*(std::vector<int>*)vals[2]).resize(num_vals[2]);
  (*(std::vector<int>*)vals[2])[0] = LIQUID_MODEM_BPSK;
  (*(std::vector<int>*)vals[2])[1] = LIQUID_MODEM_QPSK;
  (*(std::vector<int>*)vals[2])[2] = LIQUID_MODEM_QAM16;
  (*(std::vector<int>*)vals[2])[3] = LIQUID_MODEM_QAM64;

  // setup storage for rx statistics
  if (sweep_mode == SWEEP_MODE_NESTED) {
    num_data_points = 2;
    for (int i=0; i<num_sweep_params; i++)
      num_data_points *= num_vals[i];
  } else {
    num_data_points = 2*num_vals[0];
  }
  rx_stats.resize(num_data_points); 
  
  // interpret command line options
  int o;
  while ((o = getopt(argc, argv, "d:s:")) != EOF) {
    switch (o) {
      case 'd':
        dwell_time_s = atoi(optarg);
        break;
      case 's':
        settle_time_s = atoi(optarg);
        break;
    }
  }
}

// destructor
SC_Performance_Sweep_Utility::~SC_Performance_Sweep_Utility() {

  log = fopen(PERFORMANCE_SWEEP_LOG_FILE, "w");
  fprintf(log, "transmit rate,inner FEC scheme,modulation scheme,node,frames received,"
          "valid frames,EVM (dB),RSSI (dB),PER,BER,Throughput (b/s),uhd overflows\n");
  for (int i=0; i<num_data_points; i++){
    fprintf(log, "%.3e,", (*(std::vector<double>*)vals[0])[(i/24)%3]);
    fprintf(log, "%s,", fec_scheme_str[(*(std::vector<int>*)vals[1])[(i/8)%3]][0]);
    fprintf(log, "%s,", modulation_types[(*(std::vector<int>*)vals[2])[(i/2)%4]].name);
    fprintf(log, "%i,", (i%2)+1);
    fprintf(log, "%i,", rx_stats[i].frames_received);
    fprintf(log, "%i,", rx_stats[i].valid_frames);
    fprintf(log, "%.3f,", rx_stats[i].evm_dB);
    fprintf(log, "%.3f,", rx_stats[i].rssi_dB);
    fprintf(log, "%.3f,", rx_stats[i].per);
    fprintf(log, "%.3e,", rx_stats[i].ber);
    fprintf(log, "%.3e,", rx_stats[i].throughput);
    fprintf(log, "%i\n", rx_stats[i].uhd_overflows);
    printf("%i,", rx_stats[i].frames_received);
    printf("%i,", rx_stats[i].valid_frames);
    printf("%.3f,", rx_stats[i].evm_dB);
    printf("%.3f,", rx_stats[i].rssi_dB);
    printf("%.3f,", rx_stats[i].per);
    printf("%.3e,", rx_stats[i].ber);
    printf("%.3e,", rx_stats[i].throughput);
    printf("%i\n", rx_stats[i].uhd_overflows);

  }
  fprintf(log, "\n\n");
  fclose(log);  

  for (int i=0; i<num_sweep_params; i++)
    delete((std::vector<double>*)vals[i]);

}

// setup feedback enables for each node
void SC_Performance_Sweep_Utility::initialize_node_fb() {

  int fb_enables = CRTS_RX_STATS_FB_EN; 
  set_node_parameter(1, CRTS_FB_EN, (void*) &fb_enables);
  set_node_parameter(2, CRTS_FB_EN, (void*) &fb_enables);

  double rx_stats_period = dwell_time_s;
  set_node_parameter(1, CRTS_RX_STATS, (void*) &rx_stats_period); 
  set_node_parameter(1, CRTS_RX_STATS_FB, (void*) &rx_stats_period);
  set_node_parameter(2, CRTS_RX_STATS, (void*) &rx_stats_period); 
  set_node_parameter(2, CRTS_RX_STATS_FB, (void*) &rx_stats_period);

  print_sweep_point_summary();
}

// execute function
void SC_Performance_Sweep_Utility::execute() { 
  
  if ((sc_event == FEEDBACK) && (fb.fb_type == CRTS_RX_STATS)) {
    rx_stats[data_ind+fb.node-1] = *(struct ExtensibleCognitiveRadio::rx_statistics*) fb.arg;

    printf("%i,", rx_stats[data_ind+fb.node-1].frames_received);
    printf("%i,", rx_stats[data_ind+fb.node-1].valid_frames);
    printf("%.3f,", rx_stats[data_ind+fb.node-1].evm_dB);
    printf("%.3f,", rx_stats[data_ind+fb.node-1].rssi_dB);
    printf("%.3f,", rx_stats[data_ind+fb.node-1].per);
    printf("%.3e,", rx_stats[data_ind+fb.node-1].ber);
    printf("%.3e,", rx_stats[data_ind+fb.node-1].throughput);
    printf("%i\n", rx_stats[data_ind+fb.node-1].uhd_overflows);

    printf("Node %i has sent updated receive statistics:\n", fb.node);
    printf("  Number of frames received: %i\n", rx_stats[data_ind+fb.node-1].frames_received);
    printf("  Average EVM:               %.3f\n", rx_stats[data_ind+fb.node-1].evm_dB);
    printf("  Average RSSI:              %.3f\n", rx_stats[data_ind+fb.node-1].rssi_dB);
    printf("  Average PER:               %.3f\n", rx_stats[data_ind+fb.node-1].per);
    printf("  Average throughput:        %.3e\n\n", rx_stats[data_ind+fb.node-1].throughput); 
    
    if (fb.node == 1) node_1_feedback_received = true;
    if (fb.node == 2) node_2_feedback_received = true;
    
    if (node_1_feedback_received && node_2_feedback_received) {
      printf("num_data_points %i data_ind %i\n", num_data_points, data_ind);
      data_ind += 2;
      node_1_feedback_received = false;
      node_2_feedback_received = false;
      update_sweep_params();
      print_sweep_point_summary();
    }
  }
}

void SC_Performance_Sweep_Utility::print_sweep_point_summary() {

  printf("Current sweep parameters:\n");
  //for (int i=0; i<num_params; i++) {
    printf("%s: %f\n", crts_param_str[params[0]], (*(std::vector<double>*)vals[0])[indices[0]]);
    printf("%s: %s\n", crts_param_str[params[1]], 
      fec_scheme_str[(*(std::vector<int>*)vals[1])[indices[1]]][1]);
    printf("%s: %s\n\n", crts_param_str[params[2]], 
      modulation_types[(*(std::vector<int>*)vals[2])[indices[2]]].name);
  //} 
}

void SC_Performance_Sweep_Utility::update_sweep_params() {

  for (int i=num_sweep_params-1; i>=0; i--) {
    indices[i]++;
    if (indices[i] == num_vals[i]) {
      for (int j=num_sweep_params-1; j>=i; j--)
        indices[j] = 0;
    } else {
      set_params(i);
      break;
    }
  }
}

void SC_Performance_Sweep_Utility::set_params(int param_ind) {

  for (int i=param_ind; i<=num_sweep_params-1; i++) {
    switch(get_crts_param_type(params[i])) {
      case (CRTS_PARAM_INT):
        set_node_parameter(1,params[i],(void*) &((*(std::vector<int>*)vals[i])[indices[i]]));
        set_node_parameter(2,params[i],(void*) &((*(std::vector<int>*)vals[i])[indices[i]]));  
        break;
      case (CRTS_PARAM_DOUBLE):
        set_node_parameter(1,params[i],(void*) &((*(std::vector<double>*)vals[i])[indices[i]]));
        set_node_parameter(2,params[i],(void*) &((*(std::vector<double>*)vals[i])[indices[i]]));  
        if (params[i] == CRTS_TX_RATE) {
          set_node_parameter(1,CRTS_RX_RATE,(void*) &((*(std::vector<double>*)vals[i])[indices[i]]));
          set_node_parameter(2,CRTS_RX_RATE,(void*) &((*(std::vector<double>*)vals[i])[indices[i]])); 
        }
        break;
      default:
        printf("Unknown crts parameter type\n");
        exit(EXIT_FAILURE);
    }
  }

  sleep(settle_time_s);
  set_node_parameter(1,CRTS_RX_RESET,NULL);
  set_node_parameter(2,CRTS_RX_RESET,NULL);
}




