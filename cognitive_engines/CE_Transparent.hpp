#ifndef _CE_TRANSPARENT_
#define _CE_TRANSPARENT_

#include "CE.hpp"

class CE_Transparent : public Cognitive_Engine {
    public:
        CE_Transparent();
        ~CE_Transparent();
        virtual void execute(void * _args);
};

