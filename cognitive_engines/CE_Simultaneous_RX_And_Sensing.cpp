#include "CE_Simultaneous_RX_And_Sensing.hpp"
#include "ECR.hpp"
#include "timer.h"
#include <sys/time.h>
#include <complex.h>
#include <complex>

// constructor
CE_Simultaneous_RX_And_Sensing::CE_Simultaneous_RX_And_Sensing()
{
	// initialize counter to 0
	fft_counter = 0;
	
	// create timer to enable/disable sensing
	t = timer_create();
	timer_tic(t);
	
	struct timeval tv;
	gettimeofday(&tv, NULL);
	sense_time_s = tv.tv_sec;
	sense_time_us = tv.tv_usec;

	// initialize buffers to 0
	memset(buffer, 0, fft_length*sizeof(float _Complex));
	memset(buffer_F, 0, fft_length*sizeof(float _Complex));
	memset(fft_avg, 0, fft_length*sizeof(float _Complex));
	
	// create fft plan to be used for spectrum sensing
	fft = fft_create_plan(fft_length,
		reinterpret_cast<liquid_float_complex*>(buffer),
		reinterpret_cast<liquid_float_complex*>(buffer_F),
		LIQUID_FFT_FORWARD,
		0);
}

// destructor
CE_Simultaneous_RX_And_Sensing::~CE_Simultaneous_RX_And_Sensing() {}

// execute function
void CE_Simultaneous_RX_And_Sensing::execute(void * _args){
    // type cast pointer to cognitive radio object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;
    
	struct timeval tv;
	gettimeofday(&tv, NULL);

	// turn on sensing after once the required time has past
	if( (tv.tv_sec > sense_time_s) || ((tv.tv_sec == sense_time_s)&&(tv.tv_usec >= sense_time_us)) )
		ECR->set_ce_sensing(1);
	
	// handle samples
	if(ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::USRP_RX_SAMPS){
	
		fft_counter++;
		memcpy(buffer, ECR->ce_usrp_rx_buffer, ECR->ce_usrp_rx_buffer_length);	
		fft_execute(fft);
		
		for(int i=0; i<fft_length; i++){
			fft_avg[i] += cabsf(buffer[i])/fft_averaging;
		}
		
		// reset once averaging has finished
		if(fft_counter = fft_averaging){
			memset(fft_avg, 0, fft_length*sizeof(float _Complex));
			fft_counter = 0;

			// calculate next sense time
			sense_time_s = tv.tv_sec + (long int)floorf(sensing_delay_ms/1e3);
			sense_time_us = tv.tv_usec + (long int)floorf(sensing_delay_ms*1e3);
	
			// stop forwarding usrp samples to CE
			ECR->set_ce_sensing(0);

			for(int i=0; i<fft_length; i++)
				printf("FFT value %i: %f\n", i, fft_avg[i]);
			printf("\n");
		}
	}

}

