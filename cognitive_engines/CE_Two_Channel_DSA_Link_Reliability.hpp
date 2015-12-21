#ifndef _CE_TWO_CHANNEL_DSA_LINK_RELIABILITY_
#define _CE_TWO_CHANNEL_DSA_LINK_RELIABILITY_

#include "CE.hpp"

class CE_Two_Channel_DSA_Link_Reliability : public Cognitive_Engine {
public:
  CE_Two_Channel_DSA_Link_Reliability();
  ~CE_Two_Channel_DSA_Link_Reliability();
  virtual void execute(void *_args);

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
