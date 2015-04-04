#include "CE.hpp"
#include "ECR.hpp"

// custom member struct
struct CE_Example_members{
	float example_ce_metric;
};

// custom function declarations

// constructor
CE_Example::CE_Example(){
	struct CE_Example_members cm;
	cm.example_ce_metric = 15.0;
	custom_members = malloc(sizeof(struct CE_Example_members));
	memcpy(custom_members, (void *)&cm, sizeof(struct CE_Example_members));
	
}

// destructor
CE_Example::~CE_Example() {}

// execute function
void CE_Example::execute(void * _args){
	// type cast pointer to cognitive radio object
	ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;
	// type cast custom members void pointer to custom member struct
	struct CE_Example_members * cm = (struct CE_Example_members*) custom_members;	

	//printf("The example metric is now %f\n", cm->example_ce_metric);
	cm->example_ce_metric += 1.0;
}

// custom function definitions
