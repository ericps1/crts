#include <stdio.h>
#include <liquid/liquid.h>
#include "SC_Rx_Overflow_Test.hpp"

// constructor
SC_Rx_Overflow_Test::SC_Rx_Overflow_Test(int argc, char **argv) {

  // initialize parameters
  dwell_time_s = DEFAULT_DWELL_TIME_S;
  settle_time_s = DEFAULT_SETTLE_TIME_S;
  rate_ind = 0;
  mod_ind = 0;
  fec_ind = 0;
  node_1_feedback_received = false;
  node_2_feedback_received = false;

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
SC_Rx_Overflow_Test::~SC_Rx_Overflow_Test() {

  log = fopen(RX_OVERFLOW_LOG_FILE, "w");
  for (int mod_i = 0; mod_i < num_mods; mod_i++){
    fprintf(log, "%s,Node 1", modulation_types[mods[mod_i]].name);  
    for (int rate_i = 0; rate_i < num_rates; rate_i++)
      fprintf(log, ",");
    fprintf(log, "Node 2\nFEC/Rate");
    for (int rate_i = 0; rate_i < 2*num_rates; rate_i++)
      fprintf(log, ",%.3e", rates[rate_i%num_rates]);
    fprintf(log, "\n"); 
    for (int fec_i = 0; fec_i < num_fecs; fec_i++){
      fprintf(log, "%s", fec_scheme_str[fecs[fec_i]][0]); 
      for (int rate_i = 0; rate_i < 2*num_rates; rate_i++){
        fprintf(log, ",%i", uhd_overflows[rate_i/num_rates][rate_i%num_rates][mod_i][fec_i]);
      }
      fprintf(log, "\n"); 
    }
    fprintf(log, "\n");  
  }
  fprintf(log, "\n\n");
  fclose(log); 

}

// setup feedback enables for each node
void SC_Rx_Overflow_Test::initialize_node_fb() {

  int fb_enables = CRTS_RX_STATS_FB_EN; 
  set_node_parameter(1, CRTS_FB_EN, (void*) &fb_enables);
  set_node_parameter(2, CRTS_FB_EN, (void*) &fb_enables);

  double rx_stats_period = dwell_time_s;
  set_node_parameter(1, CRTS_RX_STATS, (void*) &rx_stats_period); 
  set_node_parameter(1, CRTS_RX_STATS_FB, (void*) &rx_stats_period);
  set_node_parameter(2, CRTS_RX_STATS, (void*) &rx_stats_period); 
  set_node_parameter(2, CRTS_RX_STATS_FB, (void*) &rx_stats_period);
}

// execute function
void SC_Rx_Overflow_Test::execute() { 
  
  if ((sc_event == FEEDBACK) && (fb.fb_type == CRTS_RX_STATS)) {
    rx_stats = *(struct ExtensibleCognitiveRadio::rx_statistics*) fb.arg;
    uhd_overflows[fb.node-1][rate_ind][mod_ind][fec_ind] = rx_stats.uhd_overflows;

    printf("Node: %i, Rate: %.3e, Mod: %s, FEC: %s\n", fb.node, rates[rate_ind], 
           modulation_types[mods[mod_ind]].name, fec_scheme_str[fecs[fec_ind]][0]);
    printf("  UHD rx overflows: %i\n\n", uhd_overflows[fb.node-1][rate_ind][mod_ind][fec_ind]);

    
    if (fb.node == 1) node_1_feedback_received = true;
    if (fb.node == 2) node_2_feedback_received = true;
    
    // update transmission rate, modulation, and coding scheme
    // once feedback has been received from both nodes
    if (node_1_feedback_received && node_2_feedback_received){
      
      node_1_feedback_received = false;
      node_2_feedback_received = false;

      if (fec_ind == num_fecs-1) {
        fec_ind = 0;
        if (mod_ind == num_mods-1) {
          mod_ind = 0;
          rate_ind = (rate_ind == num_rates-1) ? 0 : rate_ind+1;
          set_node_parameter(1, CRTS_TX_RATE, (void*) &rates[rate_ind]);
          set_node_parameter(2, CRTS_TX_RATE, (void*) &rates[rate_ind]);
          set_node_parameter(1, CRTS_RX_RATE, (void*) &rates[rate_ind]);
          set_node_parameter(2, CRTS_RX_RATE, (void*) &rates[rate_ind]); 
        }
        else
          mod_ind++;
        set_node_parameter(1, CRTS_TX_MOD, (void*) &mods[mod_ind]);
        set_node_parameter(2, CRTS_TX_MOD, (void*) &mods[mod_ind]);
      }
      else
        fec_ind++;
      set_node_parameter(1, CRTS_TX_FEC0, (void*) &fecs[fec_ind]);
      set_node_parameter(2, CRTS_TX_FEC0, (void*) &fecs[fec_ind]);

      // wait for state to settle and reset statistics
      sleep(settle_time_s);
      printf("Resetting receivers\n");
      set_node_parameter(1, CRTS_RX_RESET, NULL);
      set_node_parameter(2, CRTS_RX_RESET, NULL);  
      
    }
  }
}
