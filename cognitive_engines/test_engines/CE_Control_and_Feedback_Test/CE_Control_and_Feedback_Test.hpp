#ifndef _CE_CONTROL_AND_FEEDBACK_TEST_
#define _CE_CONTROL_AND_FEEDBACK_TEST_

#include "extensible_cognitive_radio.hpp"
#include "cognitive_engine.hpp"
#include "timer.h"

class CE_Control_and_Feedback_Test : public Cognitive_Engine {

private:
  // internal members used by this CE
  const float print_stats_period_s = 1.0;
  timer print_stats_timer;
  const float tx_gain_period_s = 1.0;
  const float tx_gain_increment = 1.0;
  timer tx_gain_timer;
  int frame_counter;
  int frame_errs;
  float sum_evm;
  float sum_rssi;

public:
  CE_Control_and_Feedback_Test(int argc, char **argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_Control_and_Feedback_Test();
  virtual void execute();
};

#endif
