#ifndef _CE_SUBCARRIER_ALLOC_
#define _CE_SUBCARRIER_ALLOC_

#include "CE.hpp"

class CE_Subcarrier_Alloc : public Cognitive_Engine {
    public:
        CE_Subcarrier_Alloc();
        ~CE_Subcarrier_Alloc();
        virtual void execute(void * _args);
};

