#include "CE.hpp"
#include "ECR.hpp"
#include <stdio.h>
#include <timer.h>
#include <sys/time.h>

// custom member struct
struct CE_DSA_PU_members{
	static const float freq_a = 770e6;
    static const float freq_b = 780e6;
    static const float freq_x = 870e6;
    static const float freq_y = 880e6;

    struct timeval tv;
	time_t time_s;
	time_t switch_time_s;
	int period_s;
	int first_execution;

};

// custom function declarations

// constructor
CE_DSA_PU::CE_DSA_PU(){
	//printf("Entered DSA's constructor\n");
	struct CE_DSA_PU_members cm;
	cm.period_s = 5;
	cm.first_execution = 1;
	custom_members = malloc(sizeof(struct CE_DSA_PU_members));
	memcpy(custom_members, (void *)&cm, sizeof(struct CE_DSA_PU_members));
}

// destructor
CE_DSA_PU::~CE_DSA_PU(){
	//printf("Entered DSA's destructor\n");
}

// execute function
void CE_DSA_PU::execute(void * _args){
	struct CE_DSA_PU_members * cm = (struct CE_DSA_PU_members*) custom_members;
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;

	if(ECR->CE_metrics.CE_event == ce_timeout)
		printf("Timeout!\n");
	
	gettimeofday(&cm->tv, NULL);
	

	if(cm->first_execution){
		//printf("Setting switch time and timeout\n");
		cm->switch_time_s = cm->tv.tv_sec + cm->period_s;
		ECR->set_ce_timeout_ms(100.0);
		cm->first_execution = 0;
	}
	
    if(cm->tv.tv_sec >= cm->switch_time_s){
		//printf("Switching transmit/receive frequencies\n");
		//update switch time
		cm->switch_time_s += cm->period_s;
		
		// switch tx frequency
		if(ECR->tx_params.tx_freq == cm->freq_a)
			ECR->set_tx_freq(cm->freq_b);
		else if(ECR->tx_params.tx_freq == cm->freq_b)
			ECR->set_tx_freq(cm->freq_a);
		else if(ECR->tx_params.tx_freq == cm->freq_x)
			ECR->set_tx_freq(cm->freq_y);
		else if(ECR->tx_params.tx_freq == cm->freq_y)
			ECR->set_tx_freq(cm->freq_x);
		
		// switch rx frequency
		if(ECR->rx_params.rx_freq == cm->freq_a)
			ECR->set_rx_freq(cm->freq_b);
		else if(ECR->rx_params.rx_freq == cm->freq_b)
			ECR->set_rx_freq(cm->freq_a);
		else if(ECR->rx_params.rx_freq == cm->freq_x)
			ECR->set_rx_freq(cm->freq_y);
		else if(ECR->rx_params.rx_freq == cm->freq_y)
			ECR->set_rx_freq(cm->freq_x);
			
		printf("Transmit frequency: %f\n", ECR->tx_params.tx_freq);
		printf("Receiver frequency: %f\n\n", ECR->rx_params.rx_freq);
	}

}

// custom function definitions
