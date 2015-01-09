#include <liquid/liquid.h>
#include <time.h>
#include "CR.hpp"

int main(){

    // create cognitive radio object and set parameters
    CognitiveRadio CR;
    CR.set_tx_freq(460e6f);
    CR.set_tx_rate(1e6f);
    CR.set_tx_gain_soft(1.0f);
    CR.set_tx_gain_uhd(10.0f);

    CR.set_rx_freq(450e6f);
    CR.set_rx_rate(1e6);
    CR.set_rx_gain_uhd(10.0f);

    // start CR
    CR.start_rx();
    //CR.start_tx();

    // create packet
    unsigned char header[1] = {'T'};
    unsigned int payload_len = 256;
    unsigned char payload[payload_len];
    for(unsigned int i=0; i<payload_len; i++) payload[i] = i;

    // transmit packet repeatedly
    while(true){
	// wait
	sleep(1.0);
	// transmit packet
	CR.transmit_packet(header,
	    payload,
	    payload_len);/*,
	    LIQUID_MODEM_QPSK,
	    LIQUID_FEC_NONE,
	    LIQUID_FEC_HAMMING128
	);*/
    }


    return 0;
}
