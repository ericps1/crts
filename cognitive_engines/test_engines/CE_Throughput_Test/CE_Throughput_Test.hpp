#ifndef _CE_THROUGHPUT_TEST_
#define _CE_THROUGHPUT_TEST_

#include "CE.hpp"
#include <sys/time.h>

class CE_Throughput_Test : public Cognitive_Engine {

private:
  int first_execution;
public:
  CE_Throughput_Test(int argc, char ** argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_Throughput_Test();
  virtual void execute();
};

#endif
