#ifndef _CE_TEMPLATE_
#define _CE_TEMPLATE_

#include "CE.hpp"

class CE_Template : public Cognitive_Engine {

private:
  // internal members used by this CE

public:
  CE_Template();
  ~CE_Template();
  virtual void execute(ExtensibleCognitiveRadio *ECR);
};

#endif
