#include "CE.hpp"
#include "ECR.hpp"

// custom member struct
struct CE_Transparent_members{
    float example_ce_metric;

	CE_Transparent_members(){
		example_ce_metric = 125.0;
	}
};

// custom function declarations

// constructor
CE_Transparent::CE_Transparent(){}

// destructor
CE_Transparent::~CE_Transparent() {}

// execute function
void CE_Transparent::execute(void * _args){
    // type cast pointer to cognitive radio object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;
    
	// create a static struct to maintain variables from one execution to another
	static struct CE_Transparent_members cm;
    
	/*if(ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::TIMEOUT) printf("CE execution was triggered by a timeout\n");
    else if(ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::PHY) printf("CE execution was triggered by a physical layer event\n");
   
    printf("%f\n", cm.example_ce_metric);
    cm.example_ce_metric += 1.0;*/
}

// custom function definitions
