#include "ECR.hpp"
#include "CE_Mod_Adaptation.hpp"

#if 0
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

// constructor
CE_Mod_Adaptation::CE_Mod_Adaptation() : EVM_avg(0.0), ind(0) {
  memset(EVM_buff, 0, EVM_buffer_len * sizeof(float));
}

// destructor
CE_Mod_Adaptation::~CE_Mod_Adaptation() {}

// execute function
void CE_Mod_Adaptation::execute(void *_args) {
  // type cast pointer to cognitive radio object
  ExtensibleCognitiveRadio *ECR = (ExtensibleCognitiveRadio *)_args;

  // keep track of current and desired modulation for rx and tx
  int current_tx_mod = ECR->get_tx_modulation();
  int desired_tx_mod;
  int desired_rx_mod = LIQUID_MODEM_QAM4;

  // character array to write/read control information for the ECR
  unsigned char control_info[6];

  // only update average EVM  or tx modulation for physical layer events
  if (ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::PHY) {

    // define old and new EVM values
    float EVM_old = EVM_buff[ind];
    float EVM_new;

    /*
     * Due to an invalid received frame, EVM may be a NaN.
     * If this is the case assign a proper value to indicate
     * a very bad EVM.
    */
    if (isnanf(ECR->CE_metrics.stats.evm)) {
      EVM_new = 0;
    } else {
      EVM_new = ECR->CE_metrics.stats.evm;
    }

    // update EVM history
    EVM_buff[ind] = EVM_new;

    // update moving average EVM
    EVM_avg += (EVM_new - EVM_old) / EVM_buffer_len;

    dprintf("\n------------------------------------------------\n");
    dprintf("\nNew EVM: %f\n", EVM_new);
    dprintf("Old EVM: %f\n", EVM_old);
    dprintf("Average EVM: %f\n", EVM_avg);

    // update desired receive modulation scheme based on averaged EVM
    if (EVM_avg > -15.0f) {
      dprintf("Setting desired rx modulation to QPSK\n");
      desired_rx_mod = LIQUID_MODEM_QAM4;
    } else if (EVM_avg > -25.0f) {
      dprintf("Setting desired rx modulation to 16-QAM\n");
      desired_rx_mod = LIQUID_MODEM_QAM16;
    } else if (EVM_avg > -35.0f) {
      dprintf("Setting desired rx modulation to 64-QAM\n");
      desired_rx_mod = LIQUID_MODEM_QAM64;
    }

    // set control info to update the transmitter modulation scheme
    memcpy(control_info, &desired_rx_mod, sizeof(int));
    ECR->set_tx_control_info(control_info);

    // increment the EVM buffer index and wrap around
    ind++;
    if (ind >= EVM_buffer_len)
      ind = 0;

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
