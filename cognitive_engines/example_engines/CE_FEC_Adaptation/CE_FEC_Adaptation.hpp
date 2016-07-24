#ifndef _CE_FEC_ADAPTATION_
#define _CE_FEC_ADAPTATION_

#include "extensible_cognitive_radio.hpp"
#include "cognitive_engine.hpp"
#include "include/FEC_Select.hpp"

class CE_FEC_Adaptation : public CognitiveEngine {

private:
public:
  CE_FEC_Adaptation(int argc, char ** argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_FEC_Adaptation();
  virtual void execute();
};

#endif
