#ifndef _CE_MOD_ADAPTATION_
#define _CE_MOD_ADAPTATION_

#include "CE.hpp"
#define EVM_buffer_len 3

class CE_Mod_Adaptation : public Cognitive_Engine {
private:
  float EVM_buff[EVM_buffer_len];
  float EVM_avg;
  int ind;

public:
  CE_Mod_Adaptation();
  ~CE_Mod_Adaptation();
  virtual void execute(ExtensibleCognitiveRadio *ECR);
};

#endif
