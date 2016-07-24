#ifndef _CE_THROUGHPUT_TEST_
#define _CE_THROUGHPUT_TEST_

#include <stdio.h>
#include <timer.h>
#include <sys/time.h>
#include "extensible_cognitive_radio.hpp"
#include "cognitive_engine.hpp"

class CE_Throughput_Test : public CognitiveEngine {

private:
  int first_execution;
public:
  CE_Throughput_Test(int argc, char ** argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_Throughput_Test();
  virtual void execute();
};

#endif
