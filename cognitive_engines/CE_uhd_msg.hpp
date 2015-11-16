#ifndef _CE_UHD_MSG_
#define _CE_UHD_MSG_

#include "CE.hpp"

class CE_uhd_msg : public Cognitive_Engine {
    
    private:
    public:
        CE_uhd_msg();
        ~CE_uhd_msg();
        virtual void execute(void * _args);
};

#endif
