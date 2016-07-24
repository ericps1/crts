#ifndef _CE_MOD_ADAPTATION_
#define _CE_MOD_ADAPTATION_

#include "extensible_cognitive_radio.hpp"
#include "cognitive_engine.hpp"
#include "include/Mod_Select.hpp"

class CE_Mod_Adaptation : public CognitiveEngine {
private:
public:
  CE_Mod_Adaptation(int argc, char **argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_Mod_Adaptation();
  virtual void execute();
};

#endif
