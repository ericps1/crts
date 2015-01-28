#include "CE.hpp"
#include "CR.hpp"

// custom member struct
struct CE_Example_2_members{
	float example_ce_metric;
};

// custom function declarations

// constructor
CE_Example_2::CE_Example_2(){
	printf("Entered CE example 2's constructor\n");
	struct CE_Example_2_members cm = {};
	custom_members = (void *)&cm;
}

// destructor
CE_Example_2::~CE_Example_2(){
	printf("Entered CE example 2's constructor\n");
}

// execute function
void CE_Example_2::execute(void * _args){
	struct CE_Example_2_members * cm = (struct CE_Example_2_members*) custom_members;
	printf("Entered CE example 2's execute function\n");
	printf("The example metric is now %f\n", cm->example_ce_metric);
	cm->example_ce_metric += 1.0;
}

// custom function definitions
