#include <stdio.h>
#include <uhd/usrp/multi_usrp.hpp>
#include "interferer.hpp"
#include "timer.h"

// Constructor
Interferer::Interferer(){
    uhd::device_addr_t dev_addr;
    usrp_tx = uhd::usrp::multi_usrp::make(dev_addr);

    //onTimer = timer_create();
	//dwellTimer = timer_create();
}

// Destructor
Interferer::~Interferer(){

    //timer_destroy(onTimer);
	//timer_destroy(dwellTimer);

}
