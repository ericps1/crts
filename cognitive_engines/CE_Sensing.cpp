#include "CE.hpp"
#include "ECR.hpp"
#include "timer.h"
#include <complex>

// custom member struct
struct CE_Sensing_members{
    unsigned int num_received;
    unsigned int bad_received;
    int noise_floor_measured;
    // Take 50 measurements, each 500 ms apart to 
    // measure the channel for a total of 25 seconds.
    static const unsigned int numMeasurements = 50;
    static const unsigned int measurementDelay = 500;
    timer t1;
};

// custom function declarations
void SenseSpectrum(ExtensibleCognitiveRadio* ECR, struct CE_Sensing_members* cm);
void measureNoiseFloor(ExtensibleCognitiveRadio* ECR, struct CE_Sensing_members* cm);

// constructor
CE_Sensing::CE_Sensing(){
    struct CE_Sensing_members cm;
    cm.num_received = 0;
    cm.bad_received = 0;
    cm.noise_floor_measured = 0;
    cm.t1 = timer_create();
    timer_tic(cm.t1);
    custom_members = malloc(sizeof(struct CE_Sensing_members));
    memcpy(custom_members, (void *)&cm, sizeof(struct CE_Sensing_members));
    
}

// destructor
CE_Sensing::~CE_Sensing() {}

// execute function
void CE_Sensing::execute(void * _args){
    // type cast pointer to cognitive radio object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;
    // type cast custom members void pointer to custom member struct
    struct CE_Sensing_members * cm = (struct CE_Sensing_members*) custom_members;    

    // If the noise floor hasn't been measured yet, do so now.
    if (!cm->noise_floor_measured) 
    {
        measureNoiseFloor(ECR, cm);
        cm->noise_floor_measured = 1;
    }

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
    // FIXME: This is based on CE_Hopper. 
    // It looks like this elseif is run even if the CE event is a timeout.
    // But I'm not sure it should it be run for a timeout.
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
            SenseSpectrum(ECR, cm);
            ECR->start_rx();
            timer_tic(cm->t1);
        }
        else
            printf("have not waited long enough to re-tune");
    }
}

// custom function definitions
void SenseSpectrum(ExtensibleCognitiveRadio* ECR, struct CE_Sensing_members* cm)
{
    // set up receive buffer
    const size_t max_samps_per_packet = ECR->usrp_rx->get_device()->get_max_recv_samps_per_packet();
    std::vector<std::complex<float> > buffer(max_samps_per_packet);
    
    // Sense Spectrum Samples for PU
    float noisePower = 0;
    do {
        // Get spectrum samples
        ECR->usrp_rx->get_device()->recv(
                &buffer.front(), buffer.size(), ECR->metadata_rx,
                uhd::io_type_t::COMPLEX_FLOAT32,
                uhd::device::RECV_MODE_ONE_PACKET
                );

        // Sense Spectrum Samples for PU
        // Calculate channel power
        for (unsigned int j=0; j<buffer.size(); j++)
        {
            noisePower += (buffer[j]*std::conj(buffer[j])).real();
        }

    } while (noisePower>cm->noise_floor_measured);

}

// custom function definitions
void measureNoiseFloor(ExtensibleCognitiveRadio* ECR, struct CE_Sensing_members* cm)
{
    // set up receive buffer
    const size_t max_samps_per_packet = ECR->usrp_rx->get_device()->get_max_recv_samps_per_packet();
    std::vector<std::complex<float> > buffer(max_samps_per_packet);
    
    // USRP samples are always magnitude <= 1 so measured power will always 
    // be less than this
    float noisePowerMin = buffer.size()+1;
    // Make numMeasurements measueremnts, each measurementDelay ms apart
    for (unsigned int i=0; i<cm->numMeasurements; i++)
    {
        // Get spectrum samples
        ECR->usrp_rx->get_device()->recv(
                &buffer.front(), buffer.size(), ECR->metadata_rx,
                uhd::io_type_t::COMPLEX_FLOAT32,
                uhd::device::RECV_MODE_ONE_PACKET
                );

        // Calculate channel power
        float noisePower = 0;
        for (unsigned int j=0; j<buffer.size(); j++)
        {
            noisePower += (buffer[j]*std::conj(buffer[j])).real();
        }
        // If channel power is lower than before, than 
        // consider it the new minimum noise power
        if (noisePower < noisePowerMin )
        {
            noisePowerMin = noisePower;
        }

        // Pause before measuring again
        usleep(cm->measurementDelay);
    }
    cm->noise_floor_measured = noisePowerMin;
}



