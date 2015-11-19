#include "ECR.hpp"
#include "CE_uhd_msg.hpp"

// constructor
CE_uhd_msg::CE_uhd_msg() {}

// destructor
CE_uhd_msg::~CE_uhd_msg() {}

// execute function
void CE_uhd_msg::execute(void *_args) {
  // type cast pointer to cognitive radio object
  ExtensibleCognitiveRadio *ECR = (ExtensibleCognitiveRadio *)_args;

  if (ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::UHD_OVERFLOW)
    printf("CE execution was triggered by a UHD overflow\n");

  if (ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::UHD_UNDERRUN)
    printf("CE execution was triggered by a UHD underrun\n");
}
