#ifndef _CE_TEMPLATE_
#define _CE_TEMPLATE_

#include "CE.hpp"

class CE_Template : public Cognitive_Engine {
    
    private:
        float example_ce_metric;
    public:
        CE_Template();
        ~CE_Template();
        virtual void execute(void * _args);
};

#endif
