#include <stdio.h>
#include <liquid/liquid.h>
#include "SC_BER_Sweep.hpp"

// constructor
SC_BER_Sweep::SC_BER_Sweep(int argc, char **argv) {
  tx_gain = tx_gain_min;
  rx_gain = rx_gain_min;
  feedback_count = 0;
  data_ind = 1;

  log = fopen(BER_SWEEP_LOG_FILE, "w");
  fclose(log);
}

// destructor
SC_BER_Sweep::~SC_BER_Sweep() {

  log = fopen(BER_SWEEP_LOG_FILE, "a");
  //fprintf(log, "BER(find(BER == 0)) = 0.5;\n"); 

  fprintf(log, "\nfigure;\n");
  fprintf(log, "plot(Total_gain(1,:), EVM(1,:)); hold all;\n");
  //fprintf(log, "plot(Total_gain(2,:), EVM(2,:));\n");
  fprintf(log, "title('EVM vs. Total Rx/Tx gain');\n");
  fprintf(log, "xlabel('Total gain (dB)');\n");
  fprintf(log, "ylabel('EVM');\n");
  //fprintf(log, "try\n");
  //fprintf(log, "  legend([\"Node 1\"; \"Node 2\"]);\n");
  //fprintf(log, "catch\n");
  //fprintf(log, "end_try_catch\n");
  
  fprintf(log, "\nfigure;\n");
  fprintf(log, "semilogy(Total_gain(1,:), BER(1,:)); hold all;\n");
  //fprintf(log, "semilogy(Total_gain(2,:), BER(2,:));\n");
  fprintf(log, "title('BER vs. Total Rx/Tx gain');\n");
  fprintf(log, "xlabel('Total gain (dB)');\n");
  fprintf(log, "ylabel('BER');\n");
  //fprintf(log, "try\n");
  //fprintf(log, "  legend([\"Node 1\"; \"Node 2\"]);\n");
  //fprintf(log, "catch\n");
  //fprintf(log, "end_try_catch\n");
  
  fprintf(log, "\nfigure;\n");
  fprintf(log, "plot(EVM(1,:), BER(1,:), 'xr'); hold all;\n");
  //fprintf(log, "plot(EVM(2,:), BER(2,:), 'ob');\n");
  fprintf(log, "set(gca, 'yscale', 'log');\n");
  fprintf(log, "title('BER vs. EVM');\n");
  fprintf(log, "xlabel('EVM (dB)');\n");
  fprintf(log, "ylabel('BER');\n");
  //fprintf(log, "try\n");
  //fprintf(log, "  legend([\"Node 1\"; \"Node 2\"]);\n"); 
  //fprintf(log, "catch\n");
  //fprintf(log, "end_try_catch\n");
  
  fclose(log);
}

// setup feedback enables for each node
void SC_BER_Sweep::initialize_node_fb() {
  printf("Enabling rx statistics feedback\n");
  int fb_enables = INT_MAX; 
  set_node_parameter(1, CRTS_FB_EN, (void*) &fb_enables);
  
  double rx_stats_period = dwell_time_s;
  set_node_parameter(1, CRTS_RX_STATS, (void*) &rx_stats_period); 
  set_node_parameter(1, CRTS_RX_STATS_FB, (void*) &rx_stats_period);

  // initialize gains
  set_node_parameter(1, CRTS_RX_GAIN, (void*) &rx_gain);
  set_node_parameter(2, CRTS_TX_GAIN, (void*) &tx_gain);
}

// execute function
void SC_BER_Sweep::execute() {

  printf("\n"); 
  printf("Tx gain: %f\n", tx_gain);
  printf("Rx gain: %f\n", rx_gain);

  // handle all possible feedback types
  switch (fb.fb_type) {
    case CRTS_RX_STATS:
      rx_stats = *(struct ExtensibleCognitiveRadio::rx_statistics*) fb.arg;
      printf("Node %i has sent updated receive statistics:\n", fb.node+1);
      printf("  Number of frames received: %i\n", rx_stats.frames_received);
      printf("  Average BER:               %.3e\n", rx_stats.ber);
      printf("  Average PER:               %.3f\n", rx_stats.per);
      printf("  Average throughput:        %.3e\n", rx_stats.throughput);
      printf("  Average EVM (dB):               %.3f\n", rx_stats.evm_dB);
      printf("  Average RSSI (dB):              %.3f\n", rx_stats.rssi_dB);
      
      log = fopen(BER_SWEEP_LOG_FILE, "a");
      fprintf(log, "Total_gain(%i,%i) = %f;\n", fb.node+1, data_ind, tx_gain+rx_gain);
      fprintf(log, "Frames(%i,%i) = %i;\n", fb.node+1, data_ind, rx_stats.frames_received);
      fprintf(log, "BER(%i,%i) = %f;\n", fb.node+1, data_ind, rx_stats.ber);
      fprintf(log, "PER(%i,%i) = %f;\n", fb.node+1, data_ind, rx_stats.per);
      fprintf(log, "Throughput(%i,%i) = %f;\n", fb.node+1, data_ind, rx_stats.throughput);
      fprintf(log, "EVM(%i,%i) = %f;\n", fb.node+1, data_ind, rx_stats.evm_dB);
      fprintf(log, "RSSI(%i,%i) = %f;\n", fb.node+1, data_ind, rx_stats.rssi_dB);
      fprintf(log, "\n");
      fclose(log);
      break;
  }
  
  feedback_count++;
  if (feedback_count == 1) {
    feedback_count = 0;
    data_ind++;
      
    // update transmitter and receiver gains
    if(tx_gain >= tx_gain_max)
      tx_gain = tx_gain_min;
    else
      tx_gain += tx_gain_step;

    if(rx_gain >= rx_gain_max)
      rx_gain = rx_gain_max;
    else
      rx_gain += rx_gain_step;

    set_node_parameter(2, CRTS_TX_GAIN, (void*) &tx_gain);
    set_node_parameter(2, CRTS_RX_GAIN, (void*) &rx_gain);

    // wait for state to settle and reset statistics
    usleep(settle_time_us);
    set_node_parameter(1, CRTS_RX_STATS_RESET, NULL);
  }
}



