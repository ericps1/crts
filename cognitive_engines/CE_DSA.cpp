#include "CE.hpp"
#include "CR.hpp"

// custom member struct
struct CE_DSA_members{
	float example_ce_metric;
    float freq_a = 460e6;
    float freq_b = 470e6;
    float freq_x = 480e6;
    float freq_y = 490e6;
};

// custom function declarations

// constructor
CE_DSA::CE_DSA(){
	//printf("Entered DSA's constructor\n");
	struct CE_DSA_members cm;
	cm.example_ce_metric = 15.0;
	custom_members = malloc(sizeof(struct CE_DSA_members));
	memcpy(custom_members, (void *)&cm, sizeof(struct CE_DSA_members));
}

// destructor
CE_DSA::~CE_DSA(){
	//printf("Entered DSA's constructor\n");
}

// execute function
void CE_DSA::execute(void * _args){
	struct CE_DSA_members * cm = (struct CE_DSA_members*) custom_members;

    // TODO: Check if packets received from other node are poor 
    // or not being received

    // TODO: If so, switch to other rx frequency and tell
    // other node to switch their tx frequency likewise.
    //if (current rx freq == freq_a)
    //{
    //    current_rx_freq = freq_b;
    //}
    //else if (current rx freq == freq_b)
    //{
    //    current_rx_freq = freq_a;
    //}
    //else if (current rx freq == freq_x)
    //{
    //    current_rx_freq = freq_y;
    //}
    //else if (current rx freq == freq_y)
    //{
    //    current_rx_freq = freq_x;
    //}
    
    // TODO: Set tx freq to that specified by 
    // packet received from other node

	//printf("Entered DSA's execute function\n");
	//printf("The example metric is now %f\n", cm->example_ce_metric);
	cm->example_ce_metric += 1.0;
}

// custom function definitions
