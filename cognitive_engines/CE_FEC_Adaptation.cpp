#include "CE.hpp"
#include "ECR.hpp"

// custom member struct
struct CE_FEC_Adaptation_members{
    float example_ce_metric;
    int num_received;

	CE_FEC_Adaptation_members(){
		example_ce_metric = 15.0;
    	num_received = 0;
	}
};

// custom function declarations

// constructor
CE_FEC_Adaptation::CE_FEC_Adaptation(){}

// destructor
CE_FEC_Adaptation::~CE_FEC_Adaptation() {}

// execute function
void CE_FEC_Adaptation::execute(void * _args){
    // type cast pointer to cognitive radio object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;
    
	static struct CE_FEC_Adaptation_members cm;
    
	if(ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::TIMEOUT) printf("CE execution was triggered by a timeout\n");
    else if(ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::PHY)
    { 
        if(ECR->CE_metrics.payload_valid)
            cm.num_received++;
        printf("num frames received: %i\n", cm.num_received);
        if(cm.num_received == 10)
        {
            printf("sending change frame????????????????????\n");
            unsigned char control[8] = {(unsigned char) 'f', 0, 0, 0, 0, 0, 0, 0};
            ECR->transmit_frame(control, NULL, 0);
        }
        if(cm.num_received % 5 == 0)
        {
            unsigned char control[8] = {(unsigned char) 'p', 0, 0, 0, 0, 0, 0, 0};
            unsigned char payload[50];
            for(unsigned int i = 0; i < 50; i++)
                payload[i] = 'b';
            ECR->transmit_frame(control, payload, 50);
        }
       // else
        //{
          //  unsigned char control[8] = {(unsigned char) 'p', 0, 0, 0, 0, 0, 0, 0};
       // }

        if('f' == (char)ECR->CE_metrics.control_info[0])
        {
            printf("resetting fec!!!!!!!!!!!!!!!!!!!!!!\n");
            ECR->set_tx_fec1(LIQUID_FEC_REP3);
        }
        if('p' == (char)ECR->CE_metrics.control_info[0])
        {
            printf("got p payload of length %u\n", ECR->CE_metrics.payload_len);
        }
    }    
}

// custom function definitions
