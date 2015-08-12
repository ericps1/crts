#include "CE.hpp"
#include "ECR.hpp"

// custom member struct
struct CE_Hopper_members{
	unsigned int num_received;
    static const float freq1 = 829e6;
    static const float freq2 = 833e6;
    static const float freq3 = 837e6;
    static const float freq4 = 841e6;
};

// custom function declarations

// constructor
CE_Hopper::CE_Hopper(){
	struct CE_Hopper_members cm;
	cm.num_received = 0;;
	custom_members = malloc(sizeof(struct CE_Hopper_members));
	memcpy(custom_members, (void *)&cm, sizeof(struct CE_Hopper_members));
	
}

// destructor
CE_Hopper::~CE_Hopper() {}

// execute function
void CE_Hopper::execute(void * _args){
	// type cast pointer to cognitive radio object
	ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;
    // type cast custom members void pointer to custom member struct
    struct CE_Hopper_members * cm = (struct CE_Hopper_members*) custom_members;	

    //Received control packet, switch tx freq
    if(ECR->CE_metrics.CE_event == ce_phy_event && ECR->CE_metrics.CE_frame == ce_frame_control)
    {
        if(ECR->CE_metrics.header[0] == 1)
        {
            printf("hopping to 1\n");
            ECR->set_tx_freq(cm->freq1);
        }
        else if(ECR->CE_metrics.header[0] == 2)
        {
            printf("hopping to 2\n");
            ECR->set_tx_freq(cm->freq2);
        }
        else if(ECR->CE_metrics.header[0] == 3)
        {
            printf("hopping to 3\n");
            ECR->set_tx_freq(cm->freq3);
        }
        else if(ECR->CE_metrics.header[0] == 4)
        {
            printf("hopping to 4\n");
            ECR->set_tx_freq(cm->freq4);
        }
    }
    else//Timeout or data packet received
    {
        if(ECR->CE_metrics.CE_event == ce_timeout || 
            (ECR->CE_metrics.CE_event == ce_phy_event &&
                    (!ECR->CE_metrics.header_valid || !ECR->CE_metrics.payload_valid)))
        {
            unsigned char header[8] = {0, 0, 0, 0, 0, 0, 0, 0};
            float current = ECR->get_rx_freq();
            if(current == cm->freq1)
            {
                ECR->set_rx_freq(cm->freq2);
                printf("rx on 2\n");
                header[0] = 2;
            }
            else if(current == cm->freq2)
            {
                ECR->set_rx_freq(cm->freq1);
                printf("rx on 1\n");
                header[0] = 1;
            }
            else if(current == cm->freq3)
            {
                ECR->set_rx_freq(cm->freq4);
                printf("rx on 4\n");
                header[0] = 4;
            }
            else if(current == cm->freq4)
            {
                ECR->set_rx_freq(cm->freq4);
                printf("rx on 3\n");
                header[0] = 3;
            }
            ECR->transmit_packet(header, NULL, 0);
        }
        else if(ECR->CE_metrics.CE_frame == ce_frame_data && ECR->CE_metrics.payload_valid)
        {
            cm->num_received++;
            printf("received packet %u on %f\n", cm->num_received, ECR->get_rx_freq());
        }
    }
}

// custom function definitions
