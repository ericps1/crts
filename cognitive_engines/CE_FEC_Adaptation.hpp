#ifndef _CE_FEC_ADAPTATION_
#define _CE_FEC_ADAPTATION_

#include "CE.hpp"
#define EVM_buff_len 5

class CE_FEC_Adaptation : public Cognitive_Engine {

private:
  float EVM_buff[EVM_buff_len];
  float EVM_avg;
  int ind;

public:
  CE_FEC_Adaptation();
  ~CE_FEC_Adaptation();
  virtual void execute(ExtensibleCognitiveRadio *ECR);
};

#endif
