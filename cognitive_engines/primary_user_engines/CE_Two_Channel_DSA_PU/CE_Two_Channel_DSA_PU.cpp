#include "CE_Two_Channel_DSA_PU.hpp"

// constructor
CE_Two_Channel_DSA_PU::CE_Two_Channel_DSA_PU(int argc, char **argv, ExtensibleCognitiveRadio *_ECR) {
  ECR = _ECR;
  first_execution = 1;
  period_s = 5;
}

// destructor
CE_Two_Channel_DSA_PU::~CE_Two_Channel_DSA_PU() {}

// execute function
void CE_Two_Channel_DSA_PU::execute() {
  
  gettimeofday(&tv, NULL);

  if (first_execution) {
    switch_time_s = tv.tv_sec + period_s;
    ECR->set_ce_timeout_ms(100.0);
    first_execution = 0;
  }

  if (tv.tv_sec >= switch_time_s) {
    // update switch time
    switch_time_s += period_s;

    float tx_freq = ECR->get_tx_freq();
    float rx_freq = ECR->get_rx_freq();

    // switch tx frequency
    if (tx_freq == freq_a)
      ECR->set_tx_freq(freq_b);
    else if (tx_freq == freq_b)
      ECR->set_tx_freq(freq_a);
    else if (tx_freq == freq_x)
      ECR->set_tx_freq(freq_y);
    else if (tx_freq == freq_y)
      ECR->set_tx_freq(freq_x);

    // switch rx frequency
    if (rx_freq == freq_a)
      ECR->set_rx_freq(freq_b);
    else if (rx_freq == freq_b)
      ECR->set_rx_freq(freq_a);
    else if (rx_freq == freq_x)
      ECR->set_rx_freq(freq_y);
    else if (rx_freq == freq_y)
      ECR->set_rx_freq(freq_x);

    printf("Transmit frequency: %f\n", ECR->get_tx_freq());
    printf("Receiver frequency: %f\n\n", ECR->get_rx_freq());
  }
}

// custom function definitions
