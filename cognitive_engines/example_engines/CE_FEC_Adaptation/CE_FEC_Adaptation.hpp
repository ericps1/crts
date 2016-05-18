#ifndef _CE_FEC_ADAPTATION_
#define _CE_FEC_ADAPTATION_

#include "CE.hpp"
#include "include/FEC_Select.hpp"

class CE_FEC_Adaptation : public Cognitive_Engine {

private:
public:
  CE_FEC_Adaptation(int argc, char ** argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_FEC_Adaptation();
  virtual void execute();
};

#endif
