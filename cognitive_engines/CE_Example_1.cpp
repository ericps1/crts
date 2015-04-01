#include "CE.hpp"
#include "CR.hpp"

// custom member struct
struct CE_Example_1_members{
	float example_ce_metric;
};

// custom function declarations

// constructor
CE_Example_1::CE_Example_1(){
	struct CE_Example_1_members cm;
	cm.example_ce_metric = 15.0;
	custom_members = malloc(sizeof(struct CE_Example_1_members));
	memcpy(custom_members, (void *)&cm, sizeof(struct CE_Example_1_members));
	
}

// destructor
CE_Example_1::~CE_Example_1() {}

// execute function
void CE_Example_1::execute(int timed_out, void * _args){
	// type cast pointer to cognitive radio object
	CognitiveRadio * CR = (CognitiveRadio *) _args;
	// type cast custom members void pointer to custom member struct
	struct CE_Example_1_members * cm = (struct CE_Example_1_members*) custom_members;	

	printf("The example metric is now %f\n", cm->example_ce_metric);
	cm->example_ce_metric += 1.0;
}

// custom function definitions
