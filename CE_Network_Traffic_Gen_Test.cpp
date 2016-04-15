#include "CE_Network_Traffic_Gen_Test.hpp"
#include "ECR.hpp"
#include <stdio.h>
#include <timer.h>
#include <sys/time.h>
#include "CE_Network_Traffic_Gen_Test.hpp"

// constructor
CE_Network_Traffic_Gen_Test::CE_Network_Traffic_Gen_Test() {
  period_s = 1;
  first_execution = 1;
}

// destructor
CE_Network_Traffic_Gen_Test::~CE_Network_Traffic_Gen_Test() {}

// execute function
void CE_Network_Traffic_Gen_Test::execute(ExtensibleCognitiveRadio *ECR) {
  
  gettimeofday(&tv, NULL);

  if (first_execution) {
    switch_time_s = tv.tv_sec + period_s;
    ECR->set_ce_timeout_ms(100.0);
    ECR->stop_tx();
	first_execution = 0; 
  }

  if (tv.tv_sec >= switch_time_s) {
    // update switch time
    switch_time_s += period_s;
    ECR->start_tx_for_frames(10);
  }
}
