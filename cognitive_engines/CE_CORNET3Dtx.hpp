#ifndef _CE_CORNET3DTX_
#define _CE_CORNET3DTX_

#include "CE.hpp"

class CE_CORNET3Dtx : public Cognitive_Engine {
    
    private:
        float example_ce_metric;
	char buffer[256];
    public:
        CE_CORNET3Dtx();
        ~CE_CORNET3Dtx();
        virtual void execute(void * _args);
};

#endif
