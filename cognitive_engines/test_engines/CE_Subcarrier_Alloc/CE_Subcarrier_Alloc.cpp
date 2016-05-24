#include "CE_Subcarrier_Alloc.hpp"

// constructor
CE_Subcarrier_Alloc::CE_Subcarrier_Alloc(int argc, char **argv, ExtensibleCognitiveRadio *_ECR) {
  ECR = _ECR;
  period_s = 1;
  first_execution = 1;
  alloc = 0;

  // define custom subcarrier allocation
  for (unsigned int i = 0; i < 32; i++) {
    // central band nulls
    if (i < 4 || 31 - i < 4)
      custom_alloc[i] = OFDMFRAME_SCTYPE_NULL;
    // guard band nulls
    else if (i > 12 && i < 20)
      custom_alloc[i] = OFDMFRAME_SCTYPE_NULL;
    // pilot subcarriers
    else if (i % 4 == 0)
      custom_alloc[i] = OFDMFRAME_SCTYPE_PILOT;
    // data subcarriers
    else
      custom_alloc[i] = OFDMFRAME_SCTYPE_DATA;
  }
}

// destructor
CE_Subcarrier_Alloc::~CE_Subcarrier_Alloc() {}

// execute function
void CE_Subcarrier_Alloc::execute() {
  
  gettimeofday(&tv, NULL);

  if (first_execution) {
    switch_time_s = tv.tv_sec + period_s;
    ECR->set_ce_timeout_ms(100.0);
    first_execution = 0; 
  }

  if (tv.tv_sec >= switch_time_s) {
    // update switch time
    switch_time_s += period_s;

    if (alloc == 0) {
      printf("Set subcarrier allocation to custom\n");
      ECR->set_tx_subcarrier_alloc(custom_alloc);
      ECR->set_rx_subcarrier_alloc(custom_alloc);
      alloc = 1;
    } else if (alloc == 1) {
      printf("Set subcarrier allocation to liquid-dsp default\n");
      ECR->set_tx_subcarrier_alloc(NULL);
      ECR->set_rx_subcarrier_alloc(NULL);
      alloc = 0;
    }
  }
}
