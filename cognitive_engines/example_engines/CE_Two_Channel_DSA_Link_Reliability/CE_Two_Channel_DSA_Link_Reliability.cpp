#include "CE_Two_Channel_DSA_Link_Reliability.hpp"

// constructor
CE_Two_Channel_DSA_Link_Reliability::CE_Two_Channel_DSA_Link_Reliability(
    int argc, char **argv, ExtensibleCognitiveRadio *_ECR) {
  ECR = _ECR;
  cons_invalid_payloads = 0;
  cons_invalid_control = 0;
  invalid_payloads_thresh = 3;
  invalid_control_thresh = 1;
}

// destructor
CE_Two_Channel_DSA_Link_Reliability::~CE_Two_Channel_DSA_Link_Reliability() {}

// execute function
void CE_Two_Channel_DSA_Link_Reliability::execute() {
  
  // If we recieved a frame and the payload is valid
  if ((ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::PHY_FRAME_RECEIVED) &&
      ECR->CE_metrics.payload_valid)
    cons_invalid_payloads = 0;
  else
    cons_invalid_payloads += 1;

  // If we recieved a frame and the control is valid
  if ((ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::PHY_FRAME_RECEIVED) &&
      ECR->CE_metrics.control_valid)
    cons_invalid_control = 0;
  else
    cons_invalid_control += 1;

  float current_rx_freq = ECR->get_rx_freq();
  // printf("Current RX freq: %f\n", current_rx_freq);
  // Check if packets received from other node are very poor
  // or not being received
  if (cons_invalid_payloads > invalid_payloads_thresh ||
      cons_invalid_control > invalid_control_thresh ||
      ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::TIMEOUT) {
    /*if (ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::TIMEOUT)
        //std::cout<<"Timed out without receiving any frames."<<std::endl;
    if (cons_invalid_payloads > invalid_payloads_thresh)
        //std::cout<<"Received "<<cons_invalid_payloads<<" consecutive invalid
    payloads."<<std::endl;
    if (cons_invalid_control > invalid_control_thresh)
        //std::cout<<"Received "<<cons_invalid_control<<" consecutive invalid
    control."<<std::endl;
            */

    // Reset counter to 0
    cons_invalid_payloads = 0;
    cons_invalid_control = 0;

    // Switch to other rx frequency and tell
    // other node to switch their tx frequency likewise.
    std::cout << "Switching from rx_freq=" << current_rx_freq << std::endl;
    if (current_rx_freq == freq_a) {
      current_rx_freq = freq_b;
    } else if (current_rx_freq == freq_b) {
      current_rx_freq = freq_a;
    } else if (current_rx_freq == freq_x) {
      current_rx_freq = freq_y;
    } else if (current_rx_freq == freq_y) {
      current_rx_freq = freq_x;
    }
    ECR->set_rx_freq(current_rx_freq);
    std::cout << "to rx_freq=" << current_rx_freq << std::endl;
    std::cout << std::endl;
  }

  // Put current rx freq as control info of next packet
  unsigned char control_info[6] = {};
  control_info[0] = 'f';
  std::memcpy(&control_info[1], &current_rx_freq, sizeof current_rx_freq);
  ECR->set_tx_control_info(control_info);

  // If we recieved a valid control and the fourth byte
  // is set to 'f' (signalling that the frequency is
  // specified in the control)
  if (ECR->CE_metrics.control_valid &&
      'f' == (char)ECR->CE_metrics.control_info[0]) {
    // Then set tx freq to that specified by
    // packet received from other node
    float tx_freq = ECR->get_tx_freq();
    float new_tx_freq;
    std::memcpy(&new_tx_freq, &ECR->CE_metrics.control_info[1],
                sizeof new_tx_freq);

    // printf("%c %f\n", ECR->CE_metrics.control_info[0], new_tx_freq);

    // This check ensures that the radio didn't accidentally receive it's own
    // transmission due to limited isolation between transmitter/receiver of
    // USRP
    if ((tx_freq == freq_a && new_tx_freq == freq_b) ||
        (tx_freq == freq_b && new_tx_freq == freq_a) ||
        (tx_freq == freq_x && new_tx_freq == freq_y) ||
        (tx_freq == freq_y && new_tx_freq == freq_x)) {
      ECR->set_tx_freq(new_tx_freq);
      std::cout << "Tx freq set to: " << new_tx_freq << std::endl;
    }
  }
}

// custom function definitions
