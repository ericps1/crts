#include "CE.hpp"
#include "ECR.hpp"

#if 0
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

#define EVM_buff_len 5	

// custom member struct
struct CE_AMC_members{
	float EVM_buff[EVM_buff_len];
	float EVM_avg;
	int ind;
};

// custom function declarations

// constructor
CE_AMC::CE_AMC(){
	struct CE_AMC_members cm = {0};
	custom_members = malloc(sizeof(struct CE_AMC_members));
	memcpy(custom_members, (void*)&cm, sizeof(struct CE_AMC_members));
}

// destructor
CE_AMC::~CE_AMC() {}

// execute function
void CE_AMC::execute(void * _args){
	// type cast pointer to cognitive radio object
	ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;
	// type cast custom members void pointer to custom member struct
	struct CE_AMC_members * cm = (struct CE_AMC_members*) custom_members;	

	// define old and new EVM values
	float EVM_old = cm->EVM_buff[cm->ind];
	float EVM_new = ECR->CE_metrics.stats.evm;
	
	// update EVM history
	cm->EVM_buff[cm->ind] = EVM_new;

	// update moving average EVM
	cm->EVM_avg += (EVM_new - EVM_old)/EVM_buff_len;

	dprintf("\nNew EVM: %f\n", EVM_new);
	dprintf("Old EVM: %f\n", EVM_old);
	dprintf("Average EVM: %f\n", cm->EVM_avg);

	// update modulation scheme based on averaged EVM
	if(cm->EVM_avg > -10.0f){
		dprintf("Setting modulation to QPSK\n");
		ECR->set_tx_modulation(LIQUID_MODEM_QAM4);
	}
	else if(cm->EVM_avg > -25.0f){
		dprintf("Setting modulation to 16-QAM\n");
		ECR->set_tx_modulation(LIQUID_MODEM_QAM16);
	}
	else if(cm->EVM_avg > -30.0f){
		dprintf("Setting modulation to 64-QAM\n");
		ECR->set_tx_modulation(LIQUID_MODEM_QAM64);
	}

	// increment the buffer index and wrap around
	cm->ind++;
	if(cm->ind >= EVM_buff_len)
		cm->ind = 0;
}

// custom function definitions



