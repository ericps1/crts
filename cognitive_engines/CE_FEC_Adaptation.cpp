#include "ECR.hpp"
#include "CE_FEC_Adaptation.hpp"

#if 0
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

// constructor
CE_FEC_Adaptation::CE_FEC_Adaptation() : EVM_avg(0.0), ind(0) {
  memset(EVM_buff, 0, EVM_buff_len * sizeof(float));
}

// destructor
CE_FEC_Adaptation::~CE_FEC_Adaptation() {}

// execute function
void CE_FEC_Adaptation::execute(void *_args) {
  // type cast pointer to cognitive radio object
  ExtensibleCognitiveRadio *ECR = (ExtensibleCognitiveRadio *)_args;

  // keep track of current and desired fec for rx and tx
  int current_tx_fec = ECR->get_tx_fec0();
  int desired_tx_fec;
  int desired_rx_fec = LIQUID_MODEM_QAM4;

  // character array to write/read control information for the ECR
  unsigned char control_info[6];

  // only update average EVM  or tx fec for physical layer events
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
    EVM_avg += (EVM_new - EVM_old) / EVM_buff_len;

    dprintf("\n---------------------------------------------------\n");
    dprintf("New EVM: %f\n", EVM_new);
    dprintf("Old EVM: %f\n", EVM_old);
    dprintf("Average EVM: %f\n", EVM_avg);

    // update rx fec scheme based on averaged EVM
    if (EVM_avg > -15.0f) {
      dprintf("Setting desired rx fec to rate 1/2\n");
      desired_rx_fec = LIQUID_FEC_CONV_V27;
    } else if (EVM_avg > -20.0f) {
      dprintf("Setting desired rx fec to rate 3/4\n");
      desired_rx_fec = LIQUID_FEC_CONV_V27P34;
    } else {
      dprintf("Setting desired rx fec to rate 7/8\n");
      desired_rx_fec = LIQUID_FEC_CONV_V27P78;
    }

    // set control info to update the transmitter fec scheme
    memcpy(control_info, &desired_rx_fec, sizeof(int));
    ECR->set_tx_control_info(control_info);

    // increment the EVM buffer index and wrap around
    ind++;
    if (ind >= EVM_buff_len)
      ind = 0;

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
