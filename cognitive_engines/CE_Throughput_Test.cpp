#include "CE_Throughput_Test.hpp"
#include "ECR.hpp"
#include <stdio.h>
#include <timer.h>
#include <sys/time.h>
#include "CE_Throughput_Test.hpp"

// constructor
CE_Throughput_Test::CE_Throughput_Test(int argc, char ** argv) {
  first_execution = 1;
}

// destructor
CE_Throughput_Test::~CE_Throughput_Test() {}

// execute function
void CE_Throughput_Test::execute(ExtensibleCognitiveRadio *ECR) {
  
  if (first_execution) {
    // Print the estimated network throughput (assuming perfect reception).
	  // The 256/288 factor is to account for the header added by the TUN interface.
    float phy_data_rate = ECR->get_tx_data_rate();
	  printf("Estimated network throughput: %e\n", (256.0/288.0)*phy_data_rate);
	  ECR->set_ce_timeout_ms(200.0);
	  first_execution = 0; 
  }
}
