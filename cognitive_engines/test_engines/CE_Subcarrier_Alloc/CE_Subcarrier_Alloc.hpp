#ifndef _CE_SUBCARRIER_ALLOC_
#define _CE_SUBCARRIER_ALLOC_

#include <stdio.h>
#include <timer.h>
#include <sys/time.h>
#include "extensible_cognitive_radio.hpp"
#include "cognitive_engine.hpp"

class CE_Subcarrier_Alloc : public CognitiveEngine {

private:
  struct timeval tv;
  time_t switch_time_s;
  int period_s;
  int first_execution;
  
  char custom_alloc[32];
  int alloc;

public:
  CE_Subcarrier_Alloc(int argc, char **argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_Subcarrier_Alloc();
  virtual void execute();
};

#endif
