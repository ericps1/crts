#include "ECR.hpp"
#include "CE_Template.hpp"

// constructor
CE_Template::CE_Template(int argc, char **argv) {}

// destructor
CE_Template::~CE_Template() {}

// execute function
void CE_Template::execute(ExtensibleCognitiveRadio *ECR) {
 
  switch(ECR->CE_metrics.CE_event) {
    case ExtensibleCognitiveRadio::TIMEOUT:
      // handle timeout events
      break;
    case ExtensibleCognitiveRadio::PHY:
      // handle physical layer frame reception events
      break;
    case ExtensibleCognitiveRadio::TX_COMPLETE:
      // handle transmission complete events
      break;
    case ExtensibleCognitiveRadio::UHD_OVERFLOW:
      // handle UHD overflow events
      break;
    case ExtensibleCognitiveRadio::UHD_UNDERRUN:
      // handle UHD underrun events
      break;
    case ExtensibleCognitiveRadio::USRP_RX_SAMPS:
      // handle samples received from the USRP when simultaneously
      // running the receiver and performing additional sensing
      break;
  }
}
