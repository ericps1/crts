#include "CE.hpp"
#include "CR.hpp"

// custom member struct
struct CE_DSA_members{
	float example_ce_metric;
    static const float freq_a = 781e6;
    static const float freq_b = 783e6;
    static const float freq_x = 785e6;
    static const float freq_y = 787e6;
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
    CognitiveRadio * CR = (CognitiveRadio *) _args;

    float current_rx_freq; 
    // Check if packets received from other node are poor 
    // or not being received
    if (!CR->CE_metrics.payload_valid)
    {
        // If so, switch to other rx frequency and tell
        // other node to switch their tx frequency likewise.
        current_rx_freq = CR->get_rx_freq();
        if (current_rx_freq == cm->freq_a)
        {
            current_rx_freq = cm->freq_b;
        }
        else if (current_rx_freq == cm->freq_b)
        {
            current_rx_freq = cm->freq_a;
        }
        else if (current_rx_freq == cm->freq_x)
        {
            current_rx_freq = cm->freq_y;
        }
        else if (current_rx_freq == cm->freq_y)
        {
            current_rx_freq = cm->freq_x;
        }
    }

    // TODO: Put current rx freq in header of next packet
    unsigned char * header = (unsigned char * ) &current_rx_freq;
    CR->set_header(header);
    
    // TODO: Set tx freq to that specified by 
    // packet received from other node
    float * new_tx_freq = (float *) CR->CE_metrics.header;
    CR->set_tx_freq(*new_tx_freq);

	//printf("Entered DSA's execute function\n");
	//printf("The example metric is now %f\n", cm->example_ce_metric);
	cm->example_ce_metric += 1.0;
}

// custom function definitions
