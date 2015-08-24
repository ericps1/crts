#include "CE.hpp"
#include "ECR.hpp"

// custom member struct
struct CE_Hopper_members{
	unsigned int num_received;
    static const float freq1 = 834e6;
    static const float freq2 = 838e6;
    static const float freq3 = 842e6;
    static const float freq4 = 846e6;
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
            ECR->set_tx_freq(cm->freq1);
        }
        else if(ECR->CE_metrics.header[0] == 2)
        {
            ECR->set_tx_freq(cm->freq2);
        }
        else if(ECR->CE_metrics.header[0] == 3)
        {
            ECR->set_tx_freq(cm->freq3);
        }
        else if(ECR->CE_metrics.header[0] == 4)
        {
            ECR->set_tx_freq(cm->freq4);
        }
    }
    else//Timeout or data packet received
    {
        if(ECR->CE_metrics.CE_event == ce_timeout) 
        {
            unsigned char header[8] = {0, 0, 0, 0, 0, 0, 0, 0};
            float current = ECR->get_rx_freq();
            if(current == cm->freq1)
            {
                ECR->set_rx_freq(cm->freq2);
                header[0] = 2;
            }
            else if(current == cm->freq2)
            {
                ECR->set_rx_freq(cm->freq1);
                header[0] = 1;
            }
            else if(current == cm->freq3)
            {
                ECR->set_rx_freq(cm->freq4);
                header[0] = 4;
            }
            else if(current == cm->freq4)
            {
                ECR->set_rx_freq(cm->freq3);
                header[0] = 3;
            }
            ECR->transmit_packet(header, NULL, 0);
        }
        else if(ECR->CE_metrics.CE_frame == ce_frame_data && ECR->CE_metrics.payload_valid)
        {
            cm->num_received++;
        }
    }
}

// custom function definitions
