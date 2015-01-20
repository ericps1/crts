#include<stdio.h>
#include<uhd/usrp/multi_usrp.hpp>
#include"interferer.hpp"

// Constructor
Interferer::Interferer(/*string with name of CE_execute function*/){

	// create usrp objects
    uhd::device_addr_t dev_addr;
    usrp_tx = uhd::usrp::multi_usrp::make(dev_addr);
    printf("Created UHD objects\n");
    
}

// Destructor
Interferer::~Interferer(){

}









