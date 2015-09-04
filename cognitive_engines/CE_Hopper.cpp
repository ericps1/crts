#include "CE.hpp"
#include "ECR.hpp"
#include "timer.h"

// custom member struct
struct CE_Hopper_members{
	unsigned int num_received;
    unsigned int bad_received;
    static const float freq1 = 770e6;
    static const float freq2 = 774e6;
    static const float freq3 = 870e6;
    static const float freq4 = 874e6;
    timer t1;
};

// custom function declarations
void SelectNewFrequency(ExtensibleCognitiveRadio* ECR, struct CE_Hopper_members* cm);

// constructor
CE_Hopper::CE_Hopper(){
	struct CE_Hopper_members cm;
	cm.num_received = 0;
    cm.bad_received = 0;
    cm.t1 = timer_create();
    timer_tic(cm.t1);
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
        if(ECR->CE_metrics.header[0] == 'f')
        {
            float* f_payload = (float*)ECR->CE_metrics.payload;
            float new_freq = f_payload[0];
            printf("received new frequency: %f\n", new_freq); 
            ECR->set_tx_freq(new_freq);
        }
    }
    else//Timeout or data packet received
    {
        if( !ECR->CE_metrics.header_valid ||
            !ECR->CE_metrics.payload_valid) 
        {
            cm->bad_received++;
            if(ECR->CE_metrics.CE_event == ce_timeout)
                printf("timeout, %u\n", cm->bad_received);
            else if(!ECR->CE_metrics.header_valid)
                printf("bad header, %u\n", cm->bad_received);
            else if(!ECR->CE_metrics.payload_valid)
                printf("bad payload, %u\n", cm->bad_received);
        }
        else if(ECR->CE_metrics.CE_frame == ce_frame_data && ECR->CE_metrics.payload_valid)
        {
            printf(".");
            fflush(stdout);
            cm->num_received++;
            cm->bad_received = 0;
        }
        if(cm->bad_received >= 10 || ECR->CE_metrics.CE_event == ce_timeout)
        {
            if(timer_toc(cm->t1) > 5.0)
            {
                cm->bad_received = 0;
                ECR->stop_rx();
                SelectNewFrequency(ECR, cm);
                ECR->start_rx();
                timer_tic(cm->t1);
            }
            else
                printf("have not waited long enough to retune");
        }
    }
}

// custom function definitions
void SelectNewFrequency(ExtensibleCognitiveRadio* ECR, struct CE_Hopper_members* cm)
{
    // set up receive buffer
    const size_t max_samps_per_packet = ECR->usrp_rx->get_device()->get_max_recv_samps_per_packet();
    std::vector<std::complex<float> > buffer(max_samps_per_packet);
    
    ECR->usrp_rx->get_device()->recv(
            &buffer.front(), buffer.size(), ECR->metadata_rx,
            uhd::io_type_t::COMPLEX_FLOAT32,
            uhd::device::RECV_MODE_ONE_PACKET
            );

    //Process samples in some way
    cm->bad_received = 0;
    unsigned char header[8] = {'f', 0, 0, 0, 0, 0, 0, 0};
    unsigned char payload[100];
    float current = ECR->get_rx_freq();
    if(current == cm->freq1)
    {
        ECR->set_rx_freq(cm->freq2);
        float* f_payload = (float*)payload;
        f_payload[0] = cm->freq2;
        printf("transmitting %f\n", cm->freq2);
    }
    else if(current == cm->freq2)
    {
        ECR->set_rx_freq(cm->freq1);
        float* f_payload = (float*)payload;
        f_payload[0] = cm->freq1;
        printf("transmitting %f\n", cm->freq1);
    }
    else if(current == cm->freq3)
    {
        ECR->set_rx_freq(cm->freq4);
        float* f_payload = (float*)payload;
        f_payload[0] = cm->freq4;
        printf("transmitting %f\n", cm->freq4);
    }
    else if(current == cm->freq4)
    {
        ECR->set_rx_freq(cm->freq3);
        float* f_payload = (float*)payload;
        f_payload[0] = cm->freq3;
        printf("transmitting %f\n", cm->freq3);
    }
    ECR->transmit_packet(header, payload, 100);
}
