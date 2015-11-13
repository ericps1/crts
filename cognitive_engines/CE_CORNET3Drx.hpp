#ifndef _CE_CORNET3DRX_
#define _CE_CORNET3DRX_

#include "CE.hpp"

class CE_CORNET3Drx : public Cognitive_Engine {
    
    private:
        float example_ce_metric;
	int error;
	int last;
	int good_packets;
	int bad_packets;
	float per;
    public:
        CE_CORNET3Drx();
        ~CE_CORNET3Drx();
        virtual void execute(void * _args);
};

#endif
