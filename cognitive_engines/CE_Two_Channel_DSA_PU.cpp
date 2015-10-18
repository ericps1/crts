#include "CE.hpp"
#include "ECR.hpp"
#include <stdio.h>
#include <timer.h>
#include <sys/time.h>

// constructor
CE_Two_Channel_DSA_PU::CE_Two_Channel_DSA_PU(){}

// destructor
CE_Two_Channel_DSA_PU::~CE_Two_Channel_DSA_PU(){}

// execute function
void CE_Two_Channel_DSA_PU::execute(void * _args){
	ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;

	static const float freq_a = 770e6;
    static const float freq_b = 769e6;
    static const float freq_x = 760e6;
    static const float freq_y = 759e6;

    static struct timeval tv;
	static time_t switch_time_s;
	static int period_s = 5;
	static int first_execution = 1;

	gettimeofday(&tv, NULL);

	if(first_execution){
		switch_time_s = tv.tv_sec + period_s;
		ECR->set_ce_timeout_ms(100.0);
		first_execution = 0;
	}
	
    if(tv.tv_sec >= switch_time_s){
		//update switch time
		switch_time_s += period_s;
		
		float tx_freq = ECR->get_tx_freq();
		float rx_freq = ECR-> get_rx_freq();

		// switch tx frequency
		if(tx_freq == freq_a)
			ECR->set_tx_freq(freq_b);
		else if(tx_freq == freq_b)
			ECR->set_tx_freq(freq_a);
		else if(tx_freq == freq_x)
			ECR->set_tx_freq(freq_y);
		else if(tx_freq == freq_y)
			ECR->set_tx_freq(freq_x);
		
		// switch rx frequency
		if(rx_freq == freq_a)
			ECR->set_rx_freq(freq_b);
		else if(rx_freq == freq_b)
			ECR->set_rx_freq(freq_a);
		else if(rx_freq == freq_x)
			ECR->set_rx_freq(freq_y);
		else if(rx_freq == freq_y)
			ECR->set_rx_freq(freq_x);
			
		printf("Transmit frequency: %f\n", ECR->get_tx_freq());
		printf("Receiver frequency: %f\n\n", ECR->get_rx_freq());
	}

}

// custom function definitions
