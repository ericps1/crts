#include "CE.hpp"
#include "ECR.hpp"
#include "timer.h"
#include <complex.h>
#include <complex>
#include <pthread.h>
#include <liquid/liquid.h>

#define DEBUG 0
#if DEBUG == 1
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

// custom member struct
struct CE_Sensing_members{
	// measured noise power and flag
    float noise_floor;
    int noise_floor_measured;

	// Multiplicative coeffiecient applied to meausured noise
    // power to determine channel threshold for PU occupancy.
    static const float threshold_coefficient = 275.0;

    // Number of measurements taken for noise floor
	// and time between each measurement
	static const unsigned int numMeasurements = 60;
    static const float measurementDelay_ms = 400.0;

    // How long to sense spectrum when checking noise floor
    // or for PU in milliseconds.
    static const float sensingPeriod_ms = 100.0;

    // How frequently to recheck for PU
    static const float sensingFrequency_Hz = 1.0;
   
	// Settling time for USRP
	static const float tune_settling_time_ms = 100.0;

	// time related variables
    static const float desired_timeout_ms = 10.0;
    timer t1;
    int tx_is_on;

	// counter/threshold for receiver frequency synchronization
	int no_sync_counter;
	static const int no_sync_threshold = 100;

	// frequencies used in scenario
	float fc;							// RF center frequency of USRP
	float fshift;						// DSP shift applied to reduce interference from transmitter
										// during sensing
	static const float freq_a = 770e6;  // Channel center frequencies
	static const float freq_b = 769e6;
	static const float freq_x = 760e6;
	static const float freq_y = 759e6;

	// DSP shift applied to reach the target channel's center frequency
	float rx_foff;
	float tx_foff;
	
	// USRP sample and FFT output buffers
	float _Complex buffer[512];
	float _Complex buffer_F[512];
};

// custom function declarations
int PUisPresent(ExtensibleCognitiveRadio* ECR, struct CE_Sensing_members* cm);
void measureNoiseFloor(ExtensibleCognitiveRadio* ECR, struct CE_Sensing_members* cm);

// constructor
CE_Sensing::CE_Sensing(){
	struct CE_Sensing_members cm;
    cm.t1 = timer_create();
    timer_tic(cm.t1);
	cm.tx_is_on = 1;
    cm.noise_floor = 0.0;
	cm.noise_floor_measured = 0;
	cm.no_sync_counter = 0;
	memset(cm.buffer, 0, 512*sizeof(float _Complex));
	
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
    if (cm->noise_floor_measured == 0) 
    {
        dprintf("Stopping transceiver\n");
		ECR->stop_tx();
        ECR->stop_rx();
        
		// Change rx freq to current tx freq
        float rx_freq = ECR->get_rx_freq();
        float tx_freq = ECR->get_tx_freq();
	
		if(rx_freq > 765e6){
			cm->fc = 750e6;
			cm->fshift = -15e6;
		}
		else{
			cm->fc = 780e6;
			cm->fshift = 15e6;
		}
		printf("\nfc: %.2e\n\n", cm->fc);

		cm->rx_foff = cm->fc - rx_freq;
		cm->tx_foff = -cm->fc + tx_freq;
		ECR->set_tx_freq(cm->fc, cm->fshift);
		ECR->set_rx_freq(cm->fc, -cm->tx_foff);

        dprintf("Measuring noise floor\n");	
		measureNoiseFloor(ECR, cm);
		
		// check if channel is currently occuppied
		int PUDetected = PUisPresent(ECR, cm);
        
		ECR->set_tx_freq(cm->fc, cm->tx_foff);
		//ECR->set_tx_freq(tx_freq);
		ECR->set_rx_freq(cm->fc, cm->rx_foff);

        // restart receiver
		ECR->start_rx();
		// restart transmitter if the channel is unoccupied
		if(!PUDetected){
			ECR->start_tx();
			cm->tx_is_on = 1;
		}

		// This CE should be executed immediately when run, so the timeout should 
        // be set to 0 in the scenario file. After the first run, a new timeout
        // value should be set.
        ECR->set_ce_timeout_ms(cm->desired_timeout_ms);
    
		cm->noise_floor_measured = 1;
		timer_tic(cm->t1);
	}

	
	// If it's time to sense the spectrum again
    if(timer_toc(cm->t1) > 1.0/cm->sensingFrequency_Hz)
    {
        timer_tic(cm->t1);
        
		// stop data receiver to enable spectrum sensing
		ECR->stop_rx();
        if (cm->tx_is_on)
        {
            // Pause Transmission
            ECR->stop_tx();
            cm->tx_is_on = 0;
        }
        
		// flag indicating to change to transmit frequency
		int switch_tx_freq = 0;
		
		// Change rx freq to current tx freq
        float rx_freq = cm->fc - cm->rx_foff;
        float tx_freq = cm->fc + cm->tx_foff;
		cm->rx_foff = cm->fc - rx_freq;
		cm->tx_foff = -cm->fc + tx_freq;
		ECR->set_tx_freq(cm->fc, cm->fshift);
		ECR->set_rx_freq(cm->fc, -cm->tx_foff);

        // pause to allow the frequency to settle
        while(true)
        {
            if(timer_toc(cm->t1) >= (cm->tune_settling_time_ms/1e3))
				break;
        }
		timer_tic(cm->t1);

		int PUDetected = PUisPresent(ECR, cm);
        
		// reset to original frequencies if no PU was detected
		if(!PUDetected){
		ECR->set_rx_freq(cm->fc, cm->rx_foff);
		ECR->set_tx_freq(cm->fc, cm->tx_foff);
        }
		// Check for PU on other possible tx freq.
		else {
			printf("PU detected in current channel (%-.2f), checking other channel\n", tx_freq);
			
			if(tx_freq == cm->freq_a)
				tx_freq = cm->freq_b;
			else if(tx_freq == cm->freq_b)
				tx_freq = cm->freq_a;
			else if(tx_freq == cm->freq_x)
				tx_freq = cm->freq_y;
			else if(tx_freq == cm->freq_y)
				tx_freq = cm->freq_x;

			cm->tx_foff = -cm->fc + tx_freq;	
			ECR->set_rx_freq(cm->fc, -cm->tx_foff);
			
			// pause to allow the frequency to settle
        	while(true)
        	{
            	if(timer_toc(cm->t1) >= (cm->tune_settling_time_ms/1e3))
					break;
        	}
			timer_tic(cm->t1);

			PUDetected = PUisPresent(ECR, cm);
			if(!PUDetected)
				switch_tx_freq = 1;
		
			// reset receiver frequency to current channel
			ECR->set_rx_freq(cm->fc, cm->rx_foff);
		}
		
		// restart receiver
        dprintf("Restarting receiver\n");
		ECR->start_rx();
        
		// resume transmissions if open channel
        if (!PUDetected)
        {
            dprintf("Starting transmitter\n");
			
			// switch to other channel if applicable
			if(switch_tx_freq){
				printf("Switching transmit frequency to %-.2f\n", tx_freq);
				ECR->set_tx_freq(cm->fc, cm->tx_foff);
			}

			ECR->start_tx();
            cm->tx_is_on = 1;
        }
    }

	// Receiver frequency selection based on timeouts and bad frames
	if(ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::TIMEOUT || !ECR->CE_metrics.payload_valid){
		cm->no_sync_counter++;
		if(cm->no_sync_counter >= cm->no_sync_threshold){
			float rx_freq = cm->fc - cm->rx_foff;
        	cm->no_sync_counter = 0;
			
			if(rx_freq == cm->freq_a)
				rx_freq = cm->freq_b;
			else if(rx_freq == cm->freq_b)
				rx_freq = cm->freq_a;
			else if(rx_freq == cm->freq_x)
				rx_freq = cm->freq_y;
			else if(rx_freq == cm->freq_y)
				rx_freq = cm->freq_x;
				
			printf("Switching rx freq to: %f\n", rx_freq);
			cm->rx_foff = cm->fc - rx_freq;
			ECR->set_rx_freq(cm->fc, cm->rx_foff);
		}
	}
	else //if(ECR->CE_metrics.payload_valid)
		cm->no_sync_counter = 0;

}


// Check if current channel power is signficantly higher than measured noise power.
// Return 0 if channel is empty
int PUisPresent(ExtensibleCognitiveRadio* ECR, struct CE_Sensing_members* cm)
{
    // set up receive buffers to sense for time specified by cm->sensingPeriod_ms
    const size_t max_samps_per_packet = ECR->usrp_rx->get_device()->get_max_recv_samps_per_packet();
    size_t numSensingSamples = (long unsigned int) ((cm->sensingPeriod_ms/1000.0)*ECR->get_rx_rate());
    unsigned int numFullPackets = numSensingSamples / max_samps_per_packet;
    size_t samps_per_last_packet = numSensingSamples % max_samps_per_packet;
    
	uhd::stream_cmd_t s(uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
	s.num_samps = numSensingSamples;
	s.stream_now = true;
	ECR->usrp_rx->issue_stream_cmd(s);
		
	static fftplan fft = fft_create_plan(512, 
		reinterpret_cast<liquid_float_complex*>(cm->buffer), 
		reinterpret_cast<liquid_float_complex*>(cm->buffer_F), 
		LIQUID_FFT_FORWARD, 
		0);

	// Calculate channel power
    float channelPower = 0.0;
    float _Complex X[512];
	memset(X, 0, 512*sizeof(float _Complex));
    // Get spectrum samples
    for (unsigned int j = 0; j<numFullPackets; j++)
    {
        ECR->usrp_rx->get_device()->recv(
                cm->buffer, max_samps_per_packet, ECR->metadata_rx,
                uhd::io_type_t::COMPLEX_FLOAT32,
                uhd::device::RECV_MODE_ONE_PACKET
                );

        // calculate and sum fft of zero-padded sample buffer
		fft_execute(fft);
		for(unsigned int k=0; k<512; k++)
			X[k] += cm->buffer_F[k];
	
	}
    // If number of samples in last packet is nonzero, get them as well.
    if (samps_per_last_packet)
    {
        ECR->usrp_rx->get_device()->recv(
                cm->buffer, samps_per_last_packet, ECR->metadata_rx,
                uhd::io_type_t::COMPLEX_FLOAT32,
                uhd::device::RECV_MODE_ONE_PACKET
                );

    	// calculate and sum fft of zero-padded sample buffer
		fft_execute(fft);
		for(unsigned int k=0; k<512; k++)
			X[k] += cm->buffer_F[k];

	}

	// sum absolute value of fft points (discard low frequency content)
	float M = 0.0f;
	for (unsigned int k=16; k<496; k++){
		//printf("FFT pt %i: %f\n", k, cabsf(X[k]));
		M += cabsf(X[k]);
	}
	
	channelPower = M*M;
	printf("Measured channel power: %.2e\n", channelPower);
		
	return (channelPower > cm->threshold_coefficient*cm->noise_floor);
	//if (channelPower > cm->threshold_coefficient*cm->noise_floor)
    //    return 1;
    //else
    //    return 0;
}

// Try to evaluate the noise floor power
void measureNoiseFloor(ExtensibleCognitiveRadio* ECR, struct CE_Sensing_members* cm)
{
    // set up receive buffers to sense for time specified by cm->sensingPeriod_ms
    const size_t max_samps_per_packet = ECR->usrp_rx->get_device()->get_max_recv_samps_per_packet();
    size_t numSensingSamples = (long unsigned int) ((cm->sensingPeriod_ms/1000.0)*ECR->get_rx_rate());
    unsigned int numFullPackets = numSensingSamples / max_samps_per_packet;
    size_t samps_per_last_packet = numSensingSamples % max_samps_per_packet;
    
	// static fft plan will be used for each execution
	static fftplan fft = fft_create_plan(512, 
		reinterpret_cast<liquid_float_complex*>(cm->buffer), 
		reinterpret_cast<liquid_float_complex*>(cm->buffer_F), 
		LIQUID_FFT_FORWARD, 
		0);

	// USRP samples are always magnitude <= 1 so measured power will always 
    // be less than this
    float noisePowerMin = (float)max_samps_per_packet + 1.0;
    // Make numMeasurements measueremnts, each measurementDelay_ms apart
    for (unsigned int i=0; i<cm->numMeasurements; i++)
    {
        uhd::stream_cmd_t s(uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
		s.num_samps = numSensingSamples;
		s.stream_now = true;
		ECR->usrp_rx->issue_stream_cmd(s);
		
		// Calculate channel power
        float noisePower = 0.0;
		float _Complex X[512];
		memset(X, 0, 512*sizeof(float _Complex));
        // Get spectrum samples
        for (unsigned int j = 0; j<numFullPackets; j++)
        {
            ECR->usrp_rx->get_device()->recv(
                    cm->buffer, max_samps_per_packet, ECR->metadata_rx,
                    uhd::io_type_t::COMPLEX_FLOAT32,
                    uhd::device::RECV_MODE_ONE_PACKET
                    );

            // calculate and sum fft of zero-padded sample buffer
			fft_execute(fft);
			for(unsigned int k=0; k<512; k++)
				X[k] += cm->buffer_F[k];

		}
        // If number of samples in last packet is nonzero, get them as well.
        if (samps_per_last_packet)
        {
            ECR->usrp_rx->get_device()->recv(
                    cm->buffer, samps_per_last_packet, ECR->metadata_rx,
                    uhd::io_type_t::COMPLEX_FLOAT32,
                    uhd::device::RECV_MODE_ONE_PACKET
                    );

			// calculate and sum fft of zero-padded final sample buffer
			fft_execute(fft);
			for(unsigned int k=0; k<512; k++)
				X[k] += cm->buffer_F[k];

		}

		// sum magnitude over fft (disregard low frequencies due to ZIF DC offset)
		float M = 0.0f;
		for (unsigned int k=16; k<496; k++){
			//printf("FFT pt %i: %f\n", k, cabsf(X[k]));
			M += cabsf(X[k]);
		}
		
		// square to calculate power
		noisePower = M*M;
		printf("Measured noise floor %u: %.2e\n", i, noisePower);
		
		// If channel power is lower than before, than 
        // consider it the new minimum noise power
        if (noisePower < noisePowerMin )
        {
            noisePowerMin = noisePower;
        }

        // Pause before measuring again
        while(true)
        {
            if(timer_toc(cm->t1) >= (cm->measurementDelay_ms/1e3))
				break;
        }
        timer_tic(cm->t1);
    }
    cm->noise_floor = noisePowerMin;
	
	// Lower bound the noise floor (based on experimental values)
	//if(cm->noise_floor < 5e2)
	//	cm->noise_floor = 5e2;

	printf("Measured Noise Floor: %f\n", cm->noise_floor);
}

