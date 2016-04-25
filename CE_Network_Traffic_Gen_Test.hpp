#ifndef _CE_FRAME_TX_TEST_
#define _CE_FRAME_TX_TEST_

#include "CE.hpp"
#include <sys/time.h>

class CE_Network_Traffic_Gen_Test: public Cognitive_Engine {

private:
  struct timeval tv;
  time_t switch_time_s;
  int period_s;
  int first_execution;

public:
  CE_Network_Traffic_Gen_Test();
  ~CE_Network_Traffic_Gen_Test();
  virtual void execute(ExtensibleCognitiveRadio *ECR);
};

#endif
