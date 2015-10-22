#ifndef _CE_TWO_CHANNEL_DSA_LINK_RELIABILITY_
#define _CE_TWO_CHANNEL_DSA_LINK_RELIABILITY_

#include "CE.hpp"

class CE_Two_Channel_DSA_Link_Reliability : public Cognitive_Engine {
    public:
        CE_Two_Channel_DSA_Link_Reliability();
        ~CE_Two_Channel_DSA_Link_Reliability();
        virtual void execute(void * _args);
};

