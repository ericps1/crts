#include "CE_Control_and_Feedback_Test.hpp"

// constructor
CE_Control_and_Feedback_Test::CE_Control_and_Feedback_Test(
    int argc, char **argv, ExtensibleCognitiveRadio *_ECR) {
  ECR = _ECR;
  tx_gain_timer = timer_create();
  tx_duty_cycle_timer = timer_create();
  timer_tic(tx_gain_timer);
  timer_tic(tx_duty_cycle_timer);
  tx_on = true;
}

// destructor
CE_Control_and_Feedback_Test::~CE_Control_and_Feedback_Test() {
  timer_destroy(tx_gain_timer);
  timer_destroy(tx_duty_cycle_timer);
}

// execute function
void CE_Control_and_Feedback_Test::execute() {

  float x = tx_on ? tx_duty_cycle : 1.0-tx_duty_cycle;
  if (timer_toc(tx_duty_cycle_timer) > x*tx_duty_cycle_period_s) {
    timer_tic(tx_duty_cycle_timer);
    tx_on = !tx_on;

    if(tx_on) {
      printf("Starting tx\n");
      ECR->start_tx();
    } else {
      printf("Stopping tx\n");
      ECR->stop_tx();
    }
  }

  if (tx_on && (timer_toc(tx_gain_timer) > tx_gain_period_s)) {
    timer_tic(tx_gain_timer);

    float current_tx_gain = ECR->get_tx_gain_uhd();
    float new_tx_gain = (current_tx_gain+tx_gain_increment <= 25.0) ?
      current_tx_gain + tx_gain_increment : 0.0;
    
    printf("Setting tx gain to %f dB\n", new_tx_gain);
    ECR->set_tx_gain_uhd(new_tx_gain);
  } 
}
