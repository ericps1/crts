#ifndef _CE_TWO_CHANNEL_DSA_SPECTRUM_SENSING_
#define _CE_TWO_CHANNEL_DSA_SPECTRUM_SENSING_

#include "CE.hpp"

class CE_Two_Channel_DSA_Spectrum_Sensing : public Cognitive_Engine {
    public:
        CE_Two_Channel_DSA_Spectrum_Sensing();
        ~CE_Two_Channel_DSA_Spectrum_Sensing();
        virtual void execute(void * _args);
};

