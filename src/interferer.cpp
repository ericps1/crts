#include <stdio.h>
#include <uhd/usrp/multi_usrp.hpp>
#include "interferer.hpp"

// Constructor
Interferer::Interferer(){
    uhd::device_addr_t dev_addr;
    usrp_tx = uhd::usrp::multi_usrp::make(dev_addr);
}

// Destructor
Interferer::~Interferer(){}
