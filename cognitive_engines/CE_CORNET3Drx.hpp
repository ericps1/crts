#ifndef _CE_CORNET3DRX_
#define _CE_CORNET3DRX_

#include "CE.hpp"
#include "timer.h" 

class CE_CORNET3Drx : public Cognitive_Engine {
    
    private:
        int counter;
        int good_packets, bad_packets;
        float per;
        timer feedback_timer;
        timer window_timer;
	char buffer[10];
    public:
        CE_CORNET3Drx();
        ~CE_CORNET3Drx();
        virtual void execute(ExtensibleCognitiveRadio *ECR);
};

#endif
