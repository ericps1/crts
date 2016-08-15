#include "CE_FEC_Adaptation.hpp"

#if 0
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

// constructor
CE_FEC_Adaptation::CE_FEC_Adaptation(int argc, char ** argv, ExtensibleCognitiveRadio *_ECR) {
  
  ECR = _ECR;

  double tracking_window = 0.1;
  // interpret command line options
  int o;
  while ((o = getopt(argc, argv, "w:")) != EOF) {
    switch (o) {
      case 'w':
        tracking_window = atoi(optarg);
        break;
    }
  }
  
  // track rx statistics over a 100 ms window
  ECR->set_rx_stat_tracking(true, tracking_window);
}

// destructor
CE_FEC_Adaptation::~CE_FEC_Adaptation() {}

// execute function
void CE_FEC_Adaptation::execute() {
  
  static struct ExtensibleCognitiveRadio::rx_statistics rx_stats;
  
  // keep track of current and desired fec for rx and tx
  int current_tx_fec = ECR->get_tx_fec0();
  int desired_tx_fec;
  int desired_rx_fec = LIQUID_MODEM_QAM4;

  // character array to write/read control information for the ECR
  unsigned char control_info[6];

  // only update average EVM or tx fec for physical layer events
  if (ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::PHY_FRAME_RECEIVED) {
    
    // update rx fec scheme based on averaged EVM
    rx_stats = ECR->get_rx_stats();
    desired_rx_fec = fec_select(rx_stats.evm_dB);
    
    // set control info to update the transmitter fec scheme
    memcpy(control_info, &desired_rx_fec, sizeof(int));
    ECR->set_tx_control_info(control_info);

    // obtain the desired tx fec based on the received control information
    ECR->get_rx_control_info(control_info);
    desired_tx_fec = *(int *)control_info;

    // update transmitter fec if necessary
    if (ECR->CE_metrics.control_valid && current_tx_fec != desired_tx_fec &&
        (desired_tx_fec == LIQUID_FEC_CONV_V27 ||
         desired_tx_fec == LIQUID_FEC_CONV_V27P34 ||
         desired_tx_fec == LIQUID_FEC_CONV_V27P78)) {
      dprintf("Setting tx FEC to: %i\n", desired_tx_fec);
      ECR->set_tx_fec0(desired_tx_fec);
    }
    dprintf("desired tx FEC: %i\n", desired_tx_fec);
    dprintf("current tx FEC: %i\n", current_tx_fec);
    dprintf("desired rx FEC: %i\n", desired_rx_fec);

  } else
    // for this example we assume there won't be USRP overflows or underruns
    dprintf("CE was triggered by a timeout\n");
}

// custom function definitions
