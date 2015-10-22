#ifndef _CE_TWO_CHANNEL_DSA_PU_
#define _CE_TWO_CHANNEL_DSA_PU_

#include "CE.hpp"
#include "timer.h"
#include <sys/time.h>

class CE_Two_Channel_DSA_PU : public Cognitive_Engine {
    public:
        CE_Two_Channel_DSA_PU();
        ~CE_Two_Channel_DSA_PU();
        virtual void execute(void * _args);

	private:
		static const float freq_a = 770e6;
    	static const float freq_b = 769e6;
    	static const float freq_x = 760e6;
    	static const float freq_y = 759e6;

    	struct timeval tv;
		time_t switch_time_s;
		int period_s;
		int first_execution;
};

#endif
