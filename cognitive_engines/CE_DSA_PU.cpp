#include "CE.hpp"
#include "ECR.hpp"
#include <stdio.h>
#include <timer.h>
#include <sys/time.h>

// custom member struct
struct CE_DSA_PU_members{
	static const float freq_a = 770e6;
    static const float freq_b = 769e6;
    static const float freq_x = 760e6;
    static const float freq_y = 759e6;

    struct timeval tv;
	time_t time_s;
	time_t switch_time_s;
	int period_s;
	int first_execution;

	CE_DSA_PU_members(){
		period_s = 5;
		first_execution = 1;
	}
	
};

// custom function declarations

// constructor
CE_DSA_PU::CE_DSA_PU(){}

// destructor
CE_DSA_PU::~CE_DSA_PU(){}

// execute function
void CE_DSA_PU::execute(void * _args){
	ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;

	struct CE_DSA_PU_members cm;
	
	gettimeofday(&cm.tv, NULL);
	

	if(cm.first_execution){
		cm.switch_time_s = cm.tv.tv_sec + cm.period_s;
		ECR->set_ce_timeout_ms(100.0);
		cm.first_execution = 0;
	}
	
    if(cm.tv.tv_sec >= cm.switch_time_s){
		//update switch time
		cm.switch_time_s += cm.period_s;
		
		float tx_freq = ECR->get_tx_freq();
		float rx_freq = ECR-> get_rx_freq();

		// switch tx frequency
		if(tx_freq == cm.freq_a)
			ECR->set_tx_freq(cm.freq_b);
		else if(tx_freq == cm.freq_b)
			ECR->set_tx_freq(cm.freq_a);
		else if(tx_freq == cm.freq_x)
			ECR->set_tx_freq(cm.freq_y);
		else if(tx_freq == cm.freq_y)
			ECR->set_tx_freq(cm.freq_x);
		
		// switch rx frequency
		if(rx_freq == cm.freq_a)
			ECR->set_rx_freq(cm.freq_b);
		else if(rx_freq == cm.freq_b)
			ECR->set_rx_freq(cm.freq_a);
		else if(rx_freq == cm.freq_x)
			ECR->set_rx_freq(cm.freq_y);
		else if(rx_freq == cm.freq_y)
			ECR->set_rx_freq(cm.freq_x);
			
		printf("Transmit frequency: %f\n", ECR->get_tx_freq());
		printf("Receiver frequency: %f\n\n", ECR->get_rx_freq());
	}

}

// custom function definitions
