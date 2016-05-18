#ifndef _CE_TWO_CHANNEL_DSA_PU_
#define _CE_TWO_CHANNEL_DSA_PU_

#include "CE.hpp"
#include "timer.h"
#include <sys/time.h>

class CE_Two_Channel_DSA_PU : public Cognitive_Engine {
public:
  CE_Two_Channel_DSA_PU(int argc, char**argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_Two_Channel_DSA_PU();
  virtual void execute();

private:
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
