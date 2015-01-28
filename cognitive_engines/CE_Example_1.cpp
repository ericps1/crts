#include "CE.hpp"
#include "CR.hpp"

// custom member struct
struct CE_Example_1_members{
	float example_ce_metric;
};

// custom function declarations

// constructor
CE_Example_1::CE_Example_1(){
	printf("Entered CE example 1's constructor\n");
	struct CE_Example_1_members cm = {};
	custom_members = (void *)&cm;
}

// destructor
CE_Example_1::~CE_Example_1(){
	printf("Entered CE example 1's constructor\n");
}

// execute function
void CE_Example_1::execute(void * _args){
	struct CE_Example_1_members * cm = (struct CE_Example_1_members*) custom_members;
	printf("Entered CE example 1's execute function\n");
	printf("The example metric is now %f\n", cm->example_ce_metric);
	cm->example_ce_metric += 1.0;
}

// custom function definitions
