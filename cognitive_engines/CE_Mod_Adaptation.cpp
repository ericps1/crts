#include "CE.hpp"
#include "ECR.hpp"

#if 1
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

#define EVM_buff_len 5    

// custom member struct
struct CE_Mod_Adaptation_members{
    float EVM_buff[EVM_buff_len];
    float EVM_avg;
    int ind;
};

// custom function declarations

// constructor
CE_Mod_Adaptation::CE_Mod_Adaptation(){}

// destructor
CE_Mod_Adaptation::~CE_Mod_Adaptation() {}

// execute function
void CE_Mod_Adaptation::execute(void * _args){
    // type cast pointer to cognitive radio object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;
    
	static struct CE_Mod_Adaptation_members cm;    

    // only update/validate EVM when the CE was triggered by a physical layer event
    if(ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::PHY){
        dprintf("CE was triggered by physical layer event\n");
        // define old and new EVM values
        float EVM_old = cm.EVM_buff[cm.ind];
        float EVM_new = ECR->CE_metrics.stats.evm;
    
        // update EVM history
        cm.EVM_buff[cm.ind] = EVM_new;

        // update moving average EVM
        cm.EVM_avg += (EVM_new - EVM_old)/EVM_buff_len;

        dprintf("\nNew EVM: %f\n", EVM_new);
        dprintf("Old EVM: %f\n", EVM_old);
        dprintf("Average EVM: %f\n", cm.EVM_avg);

        // update modulation scheme based on averaged EVM
        if(cm.EVM_avg > -10.0f){
            dprintf("Setting modulation to QPSK\n");
            ECR->set_tx_modulation(LIQUID_MODEM_QAM4);
        }
        else if(cm.EVM_avg > -25.0f){
            dprintf("Setting modulation to 16-QAM\n");
            ECR->set_tx_modulation(LIQUID_MODEM_QAM16);
        }
        else if(cm.EVM_avg > -30.0f){
            dprintf("Setting modulation to 64-QAM\n");
            ECR->set_tx_modulation(LIQUID_MODEM_QAM64);
        }

        // increment the buffer index and wrap around
        cm.ind++;
        if(cm.ind >= EVM_buff_len)
            cm.ind = 0;
    }
    else printf("CE was triggered by a timeout\n");
}

// custom function definitions



