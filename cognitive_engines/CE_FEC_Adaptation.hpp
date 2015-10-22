#ifndef _CE_FEC_ADAPTATION_
#define _CE_FEC_ADAPTATION_

#include "CE.hpp"

class CE_FEC_Adaptation : public Cognitive_Engine {
    public:
        CE_FEC_Adaptation();
        ~CE_FEC_Adaptation();
        virtual void execute(void * _args);
};

