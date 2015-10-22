#ifndef _CE_TWO_CHANNEL_DSA_PU_
#define _CE_TWO_CHANNEL_DSA_PU_

#include "CE.hpp"

class CE_Two_Channel_DSA_PU : public Cognitive_Engine {
    public:
        CE_Two_Channel_DSA_PU();
        ~CE_Two_Channel_DSA_PU();
        virtual void execute(void * _args);
};

