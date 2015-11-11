#ifndef _CE_CORNET3DRX_
#define _CE_CORNET3DRX_

#include "CE.hpp"

class CE_CORNET3Drx : public Cognitive_Engine {
    
    private:
        float example_ce_metric;
    public:
        CE_CORNET3Drx();
        ~CE_CORNET3Drx();
        virtual void execute(void * _args);
};

#endif
