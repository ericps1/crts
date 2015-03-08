#include <stdlib.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <string>
#include <time.h>
#include <uhd/utils/msg.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <errno.h>
#include <signal.h>
#include <complex>
#include <liquid/liquid.h>
#include "interferer.hpp"
#include "node_parameters.hpp"
#include "read_configs.hpp"

// global variables
int sig_terminate;

void Receive_command_from_controller(int *TCP_controller, Interferer * inter, struct node_parameters *np){
	// Listen to socket for message from controller
	char command_buffer[500+sizeof(struct node_parameters)];
	int rflag = recv(*TCP_controller, command_buffer, 1+sizeof(struct node_parameters), 0);
	int err = errno;
	if (rflag <= 0){
		if((err == EAGAIN) || (err == EWOULDBLOCK))
			return;
		else{
			close(*TCP_controller);
			printf("Socket failure\n");
			exit(1);
		}
	}

	// Parse command
	switch (command_buffer[0]){
	case 's': // settings for upcoming scenario
		printf("Received settings for scenario\n");
		// copy node_parameters
		memcpy(np ,&command_buffer[1], sizeof(node_parameters));
		print_node_parameters(np);

		// set interferer parameters
		inter->usrp_tx->set_tx_freq(np->tx_freq);
		inter->usrp_tx->set_tx_rate(4*np->tx_rate);
		inter->tx_rate = np->tx_rate;
		inter->usrp_tx->set_tx_gain(np->tx_gain);
		inter->int_type = np->int_type;
		inter->period = np->period;
		inter->duty_cycle = np->duty_cycle;
		break;
	case 't': // terminate program
		printf("Received termination command from controller\n");
		exit(1);
	}
	
}

void uhd_quiet(uhd::msg::type_t type, const std::string &msg){}

void help_CRTS_interferer() {
    printf("CRTS_interferer -- Start a cognitive radio interferer node. Only needs to be run explicitly when using CRTS_controller with -m option.\n");
    printf(" -h : Help.\n");
    printf(" -t : Run Time - Length of time this node will run. In seconds.\n");
    printf("      Default: 20.0 s\n");
    printf(" -a : IP Address of node running CRTS_controller.\n");
}

void terminate(int signum){
	sig_terminate = 1;
}

int main(int argc, char ** argv){

	// register signal handlers
	signal(SIGINT, terminate);
	signal(SIGQUIT, terminate);
	signal(SIGTERM, terminate);

	float run_time = 20.0f;

    // Default IP Address of Node running CRTS_controller
	char * controller_ipaddr = (char*) "192.168.1.28";

	int d;
	while((d = getopt(argc, argv, "ht:a:")) != EOF){
		switch(d){
            case 'h': help_CRTS_interferer();       return 0;
            case 't': run_time = atof(optarg);      break;
            case 'a': controller_ipaddr = optarg;   break;
		}
	}

	// Create TCP client to controller
	//unsigned int controller_port = 4444;
	int TCP_controller = socket(AF_INET, SOCK_STREAM, 0);
	if (TCP_controller < 0)
	{
		printf("ERROR: Receiver Failed to Create Client Socket\n");
		exit(EXIT_FAILURE);
	}
	// Parameters for connecting to server
	struct sockaddr_in controller_addr;
	memset(&controller_addr, 0, sizeof(controller_addr));
	controller_addr.sin_family = AF_INET;
	controller_addr.sin_addr.s_addr = inet_addr(controller_ipaddr);
	controller_addr.sin_port = htons(4444);
	
	// Attempt to connect client socket to server
	int connect_status = connect(TCP_controller, (struct sockaddr*)&controller_addr, sizeof(controller_addr));
	if (connect_status){
		printf("Failed to Connect to server.\n");
		exit(EXIT_FAILURE);
	}
	printf("Connected to server\n");
	
	uhd::msg::register_handler(&uhd_quiet);

	// Create node parameters struct and interferer object
	struct node_parameters np;
	Interferer inter;

	// Set metadata for interferer
	inter.metadata_tx.start_of_burst = false;
	inter.metadata_tx.end_of_burst = false;
	inter.metadata_tx.has_time_spec = false;

	// Read initial scenario info from controller
	Receive_command_from_controller(&TCP_controller, &inter, &np);
	fcntl(TCP_controller, F_SETFL, O_NONBLOCK);

	int iterations = (int)(run_time/inter.period);
	int samps_on = (int)(inter.period*inter.duty_cycle*4*inter.tx_rate);
	int samps_off = (int)(inter.period*(1.0-inter.duty_cycle)*4*inter.tx_rate);

	// buffer for usrp transport
	int usrp_buffer_len = 256;
	std::vector<std::complex<float> > usrp_buffer_off(usrp_buffer_len);

	/// buffer for generating transmit signals
	int tx_buffer_len = 10*usrp_buffer_len;
	std::vector<std::complex<float> > tx_buffer(samps_on);

	//printf("duty cycle: %f\n", inter.duty_cycle);
	printf("samples on: %i\n", samps_on);
	//printf("samples off: %i\n", samples_off);
	
	// variables used to modulate waveforms
	/*float symbol;
	iquid_firdes_rnyquist(LIQUID_FIRFILT_GMSKTX, K, M, beta, 0.0f, h);
	firfilt_rrrf gmsk_filt = firfilt_rrrf_create(h, h_len);
	float filtered[buffer_len];
	float theta = 0.0;			
	std::complex<float> complex_symbol;

	int ofdm_subcarriers = int(inter.tx_rate/15e3);
	*/

	// gmsk frame generator
	gmskframegen gmsk_fg = gmskframegen_create();
	
	// rrc objects
	unsigned int k = 2;
	unsigned int m = 32;
	float beta = 0.35;
	unsigned int h_len = 2*k*m+1;
	float h[h_len];
	liquid_firdes_rrcos(k, m, beta, 0.0, h);
	firfilt_crcf rrc_filt = firfilt_crcf_create(h, h_len);
	
	// ofdm objects
	unsigned int num_subcarriers = 2*(unsigned int)(np.tx_rate/30e3);
	unsigned int cp_len = 6;
	unsigned int taper_len = 4;
	unsigned char * p = NULL;
	int frame_complete = 1;
	ofdmflexframegenprops_s fgprops;
	ofdmflexframegenprops_init_default(&fgprops);
	ofdmflexframegen ofdm_fg = ofdmflexframegen_create(num_subcarriers, cp_len, taper_len, p, &fgprops);

	// header and payload for frame generators
	unsigned char header[8];
	int payload_len = 40;
	unsigned char payload[payload_len];
	
	// Loop
	sig_terminate = 0;
	for(int i=0; i<iterations; i++){
		Receive_command_from_controller(&TCP_controller, &inter, &np);
		
		printf("Interferer On\n");

		int samp_count = 0;	
		while(samp_count<samps_on){
			// modifications for the last group of samples generated
			//if(samples_remaining <= tx_buffer_len){
			//	samples_to_generate = samples_remaining;
			//	last_samples = true;
			//}
			
			int samps_to_transmit= 0;
			switch(np.int_type){
			case(CW):
				if(i==0){ 
					for(int j=0; j<usrp_buffer_len; j++){
						tx_buffer[j].real(0.5);
						tx_buffer[j].imag(0.5);
					}
				}
				samps_to_transmit = usrp_buffer_len;
				samp_count += samps_to_transmit;
				break;
			case(AWGN):
				for(int j=0; j<usrp_buffer_len; j++){
					tx_buffer[j].real(0.5*(float)rand()/(float)RAND_MAX);
					tx_buffer[j].imag(0.5*(float)rand()/(float)RAND_MAX);
				}
				samps_to_transmit = usrp_buffer_len;
				samp_count += samps_to_transmit;
				break;
			case(GMSK):{
				// generate a random header and payload
				for(int j=0; j<8; j++)
					header[j] = rand() & 0xff;
				for(int j=0; j<payload_len; j++)
					payload[j] = rand() & 0xff;
				// generate frame
				gmskframegen_assemble(gmsk_fg, header, payload, payload_len, LIQUID_CRC_NONE, LIQUID_FEC_NONE, LIQUID_FEC_NONE);
				int frame_complete = 0;
				while(!frame_complete){
					frame_complete = gmskframegen_write_samples(gmsk_fg, &tx_buffer[samps_to_transmit]);
					samps_to_transmit += k;
				}
				samp_count += samps_to_transmit;
				break;
			}
			case(RRC):{
				samps_to_transmit= tx_buffer_len;
				std::complex<float> complex_symbol;
				if(samps_on-samp_count <= tx_buffer_len){
					samps_to_transmit = samps_on-samp_count;
				}
				for(int j=0; j<samps_to_transmit; j++){
					samp_count++;
					if(j%k == 0 && samp_count<samps_on-2*h_len){
						complex_symbol.real(0.5*(float)roundf((float)rand()/(float)RAND_MAX)-0.25);
						complex_symbol.imag(0.5*(float)roundf((float)rand()/(float)RAND_MAX)-0.25);
					}
					else{
						complex_symbol.real(0.0);
						complex_symbol.imag(0.0);
					}
					firfilt_crcf_push(rrc_filt, complex_symbol);
					firfilt_crcf_execute(rrc_filt, &tx_buffer[j]);
					//printf("%f + j%f\n", usrp_buffer_on[k].real(), usrp_buffer_on[k].imag());
				}
				break;
			}
			case(OFDM):{
				// generate frame
				if(frame_complete) ofdmflexframegen_assemble(ofdm_fg, header, payload, payload_len);
				for(int j=0; j<floor(tx_buffer_len/(num_subcarriers+cp_len)); j++){
					frame_complete = ofdmflexframegen_writesymbol(ofdm_fg, &tx_buffer[j*(num_subcarriers+cp_len)]);
					samps_to_transmit += num_subcarriers+cp_len;
					if(frame_complete) break;
				}
				samp_count += samps_to_transmit;
				break;
			}
			}
			
			int tx_samp_count = 0;//samps_to_transmit;	
			int usrp_samps = usrp_buffer_len;
			while(tx_samp_count < samps_to_transmit){
				// modifications for the last group of samples generated
				if(samps_to_transmit-tx_samp_count <= usrp_buffer_len)
					usrp_samps = samps_to_transmit-tx_samp_count;
				
				inter.usrp_tx->get_device()->send(
					&tx_buffer[tx_samp_count], usrp_samps,
					inter.metadata_tx,
					uhd::io_type_t::COMPLEX_FLOAT32,
					uhd::device::SEND_MODE_FULL_BUFF
				   	//TODO: Should we set a timeout here?
				);
				
				// update number of tx samples remaining
				tx_samp_count += usrp_buffer_len;
				if(sig_terminate) break;
			}
		}
		printf("Interferer Off\n");
		int off_samp_counter = 0;	
		int usrp_samps = usrp_buffer_len;
		while(off_samp_counter < samps_off){
			// modifications for the last group of samples generated
			if(samps_off-off_samp_counter <= usrp_buffer_len)
				usrp_samps = samps_off-off_samp_counter;
				
			inter.usrp_tx->get_device()->send(
				&usrp_buffer_off[0], usrp_samps,
				inter.metadata_tx,
				uhd::io_type_t::COMPLEX_FLOAT32,
				uhd::device::SEND_MODE_FULL_BUFF
			   	//TODO: Should we set a timeout here?
			);
				
			// update number of tx samples remaining
			off_samp_counter += usrp_samps;
			if(sig_terminate) break;
		}
		if(sig_terminate) break;
	}

	printf("Sending termination message to controller\n");
	char term_message = 't';
	write(TCP_controller, &term_message, 1);
}

