#include "ECR.hpp"
#include "CE_Control_and_Feedback_Test.hpp"

// constructor
CE_Control_and_Feedback_Test::CE_Control_and_Feedback_Test(
    int argc, char **argv) {
  print_stats_timer = timer_create();
  timer_tic(print_stats_timer);
  tx_gain_timer = timer_create();
  timer_tic(tx_gain_timer);
  frame_counter = 0;
  frame_errs = 0;
  sum_evm = 0.0;
  sum_rssi = 0.0;
}

// destructor
CE_Control_and_Feedback_Test::~CE_Control_and_Feedback_Test() {
  timer_destroy(print_stats_timer);
  timer_destroy(tx_gain_timer);
}

// execute function
void CE_Control_and_Feedback_Test::execute(ExtensibleCognitiveRadio *ECR) {

  static int dc_count = 0;
  
  if (timer_toc(tx_gain_timer) > tx_gain_period_s) {
    timer_tic(tx_gain_timer);

    float current_tx_gain = ECR->get_tx_gain_uhd();
    if(current_tx_gain < 25.0)
      ECR->set_tx_gain_uhd(current_tx_gain + tx_gain_increment);
    else
      ECR->set_tx_gain_uhd(0.0);

    dc_count++;
    if(dc_count == 3){
      dc_count = 0;
      if(ECR->get_tx_state() == TX_STOPPED){
        ECR->start_tx();
      }else{
        ECR->stop_tx();
      }
    }
  }

  if (timer_toc(print_stats_timer) > print_stats_period_s) {
    timer_tic(print_stats_timer);
    
    if (frame_counter>0) {
      printf("Updated Received Frame Statistics:\n");
      printf("  Frames Received: %i\n", frame_counter);
      printf("  Average EVM:     %f\n", 10.0*log10(sum_evm/(float)frame_counter));
      printf("  Average PER:     %f\n", (float)frame_errs/(float)frame_counter);
      printf("  Average RSSI:    %f\n\n", 10.0*log10(sum_rssi/(float)frame_counter));

      // reset statistics
      frame_counter = 0;
      frame_errs = 0;
      sum_evm = 0.0;
      sum_rssi = 0.0;
    } else {
      printf("Updated Received Frame Statistics:\n");
      printf("  Frames Received: 0\n");
      printf("  Average EVM:     -\n");
      printf("  Average PER:     -\n");
      printf("  Average RSSI:    -\n");
    }
  }

  switch(ECR->CE_metrics.CE_event) {
    case ExtensibleCognitiveRadio::TIMEOUT:
      // handle timeout events
      break;
    case ExtensibleCognitiveRadio::PHY:
      // handle physical layer frame reception events
      frame_counter++;
      if (!ECR->CE_metrics.payload_valid)
        frame_errs++;
      sum_evm += pow(10.0, ECR->CE_metrics.stats.evm/10.0);
      sum_rssi += pow(10.0, ECR->CE_metrics.stats.rssi/10.0);
      break;
    case ExtensibleCognitiveRadio::TX_COMPLETE:
      // handle transmission complete events
      break;
    case ExtensibleCognitiveRadio::UHD_OVERFLOW:
      // handle UHD overflow events
      break;
    case ExtensibleCognitiveRadio::UHD_UNDERRUN:
      // handle UHD underrun events
      break;
    case ExtensibleCognitiveRadio::USRP_RX_SAMPS:
      // handle samples received from the USRP when simultaneously
      // running the receiver and performing additional sensing
      break;
  }
}
