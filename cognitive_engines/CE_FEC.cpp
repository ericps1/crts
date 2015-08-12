#include "CE.hpp"
#include "ECR.hpp"

// custom member struct
struct CE_FEC_members{
	float example_ce_metric;
    int num_received;
};

// custom function declarations

// constructor
CE_FEC::CE_FEC(){
	struct CE_FEC_members cm;
	cm.example_ce_metric = 15.0;
    cm.num_received = 0;
	custom_members = malloc(sizeof(struct CE_FEC_members));
	memcpy(custom_members, (void *)&cm, sizeof(struct CE_FEC_members));
	
}

// destructor
CE_FEC::~CE_FEC() {}

// execute function
void CE_FEC::execute(void * _args){
	// type cast pointer to cognitive radio object
	ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;
	// type cast custom members void pointer to custom member struct
	struct CE_FEC_members * cm = (struct CE_FEC_members*) custom_members;	

	if(ECR->CE_metrics.CE_event == ce_timeout) printf("CE execution was triggered by a timeout\n");
	else if(ECR->CE_metrics.CE_event == ce_phy_event)
	{ 
        if(ECR->CE_metrics.payload_valid)
            cm->num_received++;
        printf("num packets received: %i\n", cm->num_received);
        if(cm->num_received == 10)
        {
            printf("sending change packet????????????????????\n");
            unsigned char header[8] = {(unsigned char) 'f', 0, 0, 0, 0, 0, 0, 0};
            ECR->transmit_packet(header, NULL, 0);
        }
        if(cm->num_received % 5 == 0)
        {
            unsigned char header[8] = {(unsigned char) 'p', 0, 0, 0, 0, 0, 0, 0};
            unsigned char payload[50];
            for(unsigned int i = 0; i < 50; i++)
                payload[i] = 'b';
            ECR->transmit_packet(header, payload, 50);
        }
       // else
        //{
          //  unsigned char header[8] = {(unsigned char) 'p', 0, 0, 0, 0, 0, 0, 0};
       // }

        if('f' == (char)ECR->CE_metrics.header[0])
        {
            printf("resetting fec!!!!!!!!!!!!!!!!!!!!!!\n");
            ECR->set_tx_fec1(LIQUID_FEC_REP3);
        }
        if('p' == (char)ECR->CE_metrics.header[0])
        {
            printf("got p payload of length %u\n", ECR->CE_metrics.payload_len);
        }
    }	
}

// custom function definitions
