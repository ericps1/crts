#include "CE.hpp"
#include "ECR.hpp"
#include <stdio.h>

// custom member struct
struct CE_DSA_members{
    static const float freq_a = 770e6;
    static const float freq_b = 780e6;
    static const float freq_x = 870e6;
    static const float freq_y = 880e6;

    // Number of consecutive invalid headers or payloads
    int cons_invalid_payloads;
    int cons_invalid_headers;

    // Theshold for number of consecutive invalid headers
    int invalid_payloads_thresh;
    int invalid_headers_thresh;
};

// custom function declarations

// constructor
CE_DSA::CE_DSA(){
    struct CE_DSA_members cm;
    cm.cons_invalid_payloads = 0;
    cm.cons_invalid_headers = 0;
    cm.invalid_payloads_thresh = 2;
    cm.invalid_headers_thresh = 1;
    custom_members = malloc(sizeof(struct CE_DSA_members));
    memcpy(custom_members, (void *)&cm, sizeof(struct CE_DSA_members));
}

// destructor
CE_DSA::~CE_DSA(){
    
}

// execute function
void CE_DSA::execute(void * _args){
    struct CE_DSA_members * cm = (struct CE_DSA_members*) custom_members;
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;

    // If we recieved a frame and the payload is valid
    if((ECR->CE_metrics.CE_event != ExtensibleCognitiveRadio::TIMEOUT) && ECR->CE_metrics.payload_valid)
        cm->cons_invalid_payloads = 0;
    else
        cm->cons_invalid_payloads += 1;

    // If we recieved a frame and the header is valid
    if((ECR->CE_metrics.CE_event != ExtensibleCognitiveRadio::TIMEOUT) && ECR->CE_metrics.header_valid)
        cm->cons_invalid_headers = 0;
    else
        cm->cons_invalid_headers += 1;
    
    float current_rx_freq = ECR->get_rx_freq();
    //printf("Current RX freq: %f\n", current_rx_freq);
    // Check if packets received from other node are very poor 
    // or not being received
    if (cm->cons_invalid_payloads>cm->invalid_payloads_thresh || 
        cm->cons_invalid_headers>cm->invalid_headers_thresh || 
        ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::TIMEOUT)
    {
        if (ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::TIMEOUT)
            std::cout<<"Timed out without receiving any frames."<<std::endl;
        if (cm->cons_invalid_payloads > cm->invalid_payloads_thresh)
            std::cout<<"Received "<<cm->cons_invalid_payloads<<" consecutive invalid payloads."<<std::endl;
        if (cm->cons_invalid_headers > cm->invalid_headers_thresh)
            std::cout<<"Received "<<cm->cons_invalid_headers<<" consecutive invalid headers."<<std::endl;

        // Reset counter to 0
        cm->cons_invalid_payloads = 0;
        cm->cons_invalid_headers = 0;

        // Switch to other rx frequency and tell
        // other node to switch their tx frequency likewise.
        std::cout<<"Switching from rx_freq="<<current_rx_freq<<std::endl;
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
        ECR->set_rx_freq(current_rx_freq);
        std::cout<<"to rx_freq="<<current_rx_freq<<std::endl;
        std::cout<<std::endl;
    
    }
    
    // Put current rx freq in header of next packet
    unsigned char header[8] = {};
    header[3] = 'f';
       std::memcpy(&header[4], &current_rx_freq, sizeof current_rx_freq);
    ECR->set_header(header);
   
    // If we recieved a valid header and the fourth byte 
    // is set to 'f' (signalling that the frequency is 
    // specified in the header)
    if(ECR->CE_metrics.header_valid && 'f' == (char) ECR->CE_metrics.header[3])
    {
        // Then set tx freq to that specified by 
        // packet received from other node
        float tx_freq = ECR->get_tx_freq();
        float new_tx_freq;
        std::memcpy(&new_tx_freq, &ECR->CE_metrics.header[4], sizeof new_tx_freq);
        
        // This check ensures that the radio didn't accidentally receive it's own transmission
        // due to limited isolation between transmitter/receiver of USRP
        if( (tx_freq == cm->freq_a && new_tx_freq == cm->freq_b) || 
            (tx_freq == cm->freq_b && new_tx_freq == cm->freq_a) ||
            (tx_freq == cm->freq_x && new_tx_freq == cm->freq_y) || 
            (tx_freq == cm->freq_y && new_tx_freq == cm->freq_x) )
        {    
            ECR->set_tx_freq(new_tx_freq);
            std::cout<<"Tx freq set to: "<< new_tx_freq << std::endl;
        }
    }
}

// custom function definitions
