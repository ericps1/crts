#include "ECR.hpp"
#include "CE_Template.hpp"

// constructor
CE_Template::CE_Template()
{
    example_ce_metric = 125.0;
}

// destructor
CE_Template::~CE_Template() {}

// execute function
void CE_Template::execute(void * _args){
    // type cast pointer to cognitive radio object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;
    
	/*if(ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::TIMEOUT) printf("CE execution was triggered by a timeout\n");
    else if(ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::PHY) printf("CE execution was triggered by a physical layer event\n");
   
    printf("%f\n", example_ce_metric);
    example_ce_metric += 1.0;*/
}

