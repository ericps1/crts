#include <liquid/liquid.h>
#include <time.h>
#include <getopt.h>
#include "CR.hpp"

int main(int argc, char ** argv){

	float freq_rx = 460e6;
	float freq_tx = 450e6;

	int d;
	while((d = getopt(argc,argv,"f:F:")) != EOF){
		switch (d){
		case 'f': freq_rx = 1e6*atof(optarg); break;
		case 'F': freq_tx = 1e6*atof(optarg); break;
		}
	}

    // create cognitive radio object and set parameters
    CognitiveRadio CR;
    CR.set_tx_freq(freq_tx);
    CR.set_tx_rate(1e6f);
    CR.set_tx_gain_soft(1.0f);
    CR.set_tx_gain_uhd(25.0f);

    CR.set_rx_freq(freq_rx);
    CR.set_rx_rate(1e6);
    CR.set_rx_gain_uhd(25.0f);
	CR.PHY_metrics = true;

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
		sleep(2.0);
		// transmit packet
		CR.transmit_packet(header,
	    	payload,
	    	payload_len);
    }


    return 0;
}
