#ifndef _CE_MOD_ADAPTATION_
#define _CE_MOD_ADAPTATION_

#include "CE.hpp"

class CE_Mod_Adaptation : public Cognitive_Engine {
    public:
        CE_Mod_Adaptation();
        ~CE_Mod_Adaptation();
        virtual void execute(void * _args);
};

#endif
