#ifndef _CE_TEMPLATE_
#define _CE_TEMPLATE_

#include "CE.hpp"

class CE_Template : public Cognitive_Engine {

private:
  // internal members used by this CE
  int debugLevel;

public:
  CE_Template(int argc, char **argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_Template();
  virtual void execute();
};

#endif
