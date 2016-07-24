#ifndef _CE_TWO_CHANNEL_DSA_PU_
#define _CE_TWO_CHANNEL_DSA_PU_

#include <stdio.h>
#include <sys/time.h>
#include "extensible_cognitive_radio.hpp"
#include "cognitive_engine.hpp"
#include "timer.h"

class CE_Two_Channel_DSA_PU : public CognitiveEngine {
public:
  CE_Two_Channel_DSA_PU(int argc, char**argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_Two_Channel_DSA_PU();
  virtual void execute();

private:

  /*
  //Jason
  static constexpr float freq_a = 774e6;
  static constexpr float freq_b = 766e6;
  static constexpr float freq_x = 768e6;
  static constexpr float freq_y = 772e6;
  */

  //Eric
  static constexpr float freq_a = 770e6;
  static constexpr float freq_b = 769e6;
  static constexpr float freq_x = 765e6;
  static constexpr float freq_y = 764e6;
  
  struct timeval tv;
  time_t switch_time_s;
  int period_s;
  int first_execution;
};

#endif
