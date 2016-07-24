#ifndef _CE_CONTROL_AND_FEEDBACK_TEST_
#define _CE_CONTROL_AND_FEEDBACK_TEST_

#include "extensible_cognitive_radio.hpp"
#include "cognitive_engine.hpp"
#include "timer.h"

class CE_Control_and_Feedback_Test : public CognitiveEngine {

private:
  // internal members used by this CE
  timer tx_gain_timer;
  timer tx_duty_cycle_timer;
  const float tx_gain_period_s = 5.0;
  const float tx_gain_increment = 5.0;
  static constexpr float tx_duty_cycle_period_s = 20.0;
  static constexpr float tx_duty_cycle = 0.5;
  bool tx_on;

public:
  CE_Control_and_Feedback_Test(int argc, char **argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_Control_and_Feedback_Test();
  virtual void execute();
};

#endif
