#include "CE_Mod_Adaptation.hpp"

#if 0
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

// constructor
CE_Mod_Adaptation::CE_Mod_Adaptation(int argc, char **argv, ExtensibleCognitiveRadio *_ECR){  
  ECR = _ECR;

  // track rx statistics over a 100 ms window
  ECR->set_rx_stat_tracking(true, 0.1);
}

// destructor
CE_Mod_Adaptation::~CE_Mod_Adaptation() {}

// execute function
void CE_Mod_Adaptation::execute() {
  
  static struct ExtensibleCognitiveRadio::rx_statistics rx_stats; 
  
  // keep track of current and desired modulation for rx and tx
  int current_tx_mod = ECR->get_tx_modulation();
  int desired_tx_mod;
  int desired_rx_mod = LIQUID_MODEM_QAM4;

  // character array to write/read control information for the ECR
  unsigned char control_info[6];

  // only update average EVM  or tx modulation for physical layer events
  if (ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::PHY_FRAME_RECEIVED) {

    // update desired receive modulation scheme based on averaged EVM
    rx_stats = ECR->get_rx_stats();
    desired_rx_mod = mod_select(rx_stats.evm_dB);
    
    // set control info to update the transmitter modulation scheme
    memcpy(control_info, &desired_rx_mod, sizeof(int));
    ECR->set_tx_control_info(control_info);

    // obtain the desired tx modulation based on the received control
    // information
    ECR->get_rx_control_info(control_info);
    desired_tx_mod = *(int *)control_info;
    dprintf("Desired tx mod: %i\n", desired_tx_mod);

    // update transmitter modulation if necessary
    if (ECR->CE_metrics.control_valid && current_tx_mod != desired_tx_mod &&
        desired_tx_mod >= LIQUID_MODEM_QAM4 &&
        desired_tx_mod <= LIQUID_MODEM_QAM64) {
      dprintf("Setting tx modulation\n");
      ECR->set_tx_modulation(desired_tx_mod);
    }

  } else
    // for this example we assume there won't be any USRP overflows or underruns
    dprintf("CE was triggered by a timeout\n");
}

// custom function definitions
