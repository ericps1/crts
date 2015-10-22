#ifndef _CE_MOD_ADAPTATION_
#define _CE_MOD_ADAPTATION_

#include "CE.hpp"

class CE_Mod_Adaption : public Cognitive_Engine {
    public:
        CE_Mod_Adaption();
        ~CE_Mod_Adaption();
        virtual void execute(void * _args);
};

