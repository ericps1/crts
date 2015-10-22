#include "CE_Subcarrier_Alloc.hpp"
#include "ECR.hpp"
#include <stdio.h>
#include <timer.h>
#include <sys/time.h>

// constructor
CE_Subcarrier_Alloc::CE_Subcarrier_Alloc(){}

// destructor
CE_Subcarrier_Alloc::~CE_Subcarrier_Alloc(){}

// execute function
void CE_Subcarrier_Alloc::execute(void * _args){
	ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;

	static struct timeval tv;
	static time_t switch_time_s;
	static int period_s = 5;
	static int first_execution = 1;

	static char custom_alloc[32];
	static int alloc = 0;

	gettimeofday(&tv, NULL);

	if(first_execution){
		switch_time_s = tv.tv_sec + period_s;
		ECR->set_ce_timeout_ms(100.0);
		first_execution = 0;

		// define custom subcarrier allocation
		for(unsigned int i=0; i<32; i++){
			// central band nulls
			if(i<4 || 31-i<4)
				custom_alloc[i] = OFDMFRAME_SCTYPE_NULL;
			// guard band nulls
			else if(i > 12 && i < 20)
				custom_alloc[i] = OFDMFRAME_SCTYPE_NULL;
			// pilot subcarriers
			else if(i%4 == 0)
				custom_alloc[i] = OFDMFRAME_SCTYPE_PILOT;
			// data subcarriers
			else
				custom_alloc[i] = OFDMFRAME_SCTYPE_DATA;
		}
	}
	
    if(tv.tv_sec >= switch_time_s){
		//update switch time
		switch_time_s += period_s;
		
		if(alloc == 0){
            printf("Set subcarrier allocation to custom\n");  
		    ECR->set_tx_subcarrier_alloc(custom_alloc);
		    alloc = 1;
		}
		else if(alloc == 1){
            printf("Set subcarrier allocation to liquid-dsp default\n");
		    ECR->set_tx_subcarrier_alloc(NULL);
		    alloc = 0;
		}
		
	}

}

// custom function definitions
