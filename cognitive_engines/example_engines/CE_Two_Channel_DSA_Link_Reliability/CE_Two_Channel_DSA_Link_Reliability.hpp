#ifndef _CE_TWO_CHANNEL_DSA_LINK_RELIABILITY_
#define _CE_TWO_CHANNEL_DSA_LINK_RELIABILITY_

#include <stdio.h>
#include <iostream>
#include "extensible_cognitive_radio.hpp"
#include "cognitive_engine.hpp"

class CE_Two_Channel_DSA_Link_Reliability : public CognitiveEngine {
public:
  CE_Two_Channel_DSA_Link_Reliability(int argc, char **argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_Two_Channel_DSA_Link_Reliability();
  virtual void execute();

private:
  static constexpr float freq_a = 770e6;
  static constexpr float freq_b = 769e6;
  static constexpr float freq_x = 870e6;
  static constexpr float freq_y = 869e6;

  // Number of consecutive invalid control or payloads
  int cons_invalid_payloads;
  int cons_invalid_control;

  // Theshold for number of consecutive invalid control
  int invalid_payloads_thresh;
  int invalid_control_thresh;
};

#endif
