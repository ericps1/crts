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
#include <complex>
#include <uhd/utils/msg.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include "interferer.hpp"
#include "node_parameters.hpp"
#include "read_configs.hpp"

void Receive_command_from_controller(int *TCP_controller, Interferer * inter, struct node_parameters *np){
	// Listen to socket for message from controller
	char command_buffer[500+sizeof(struct node_parameters)];
	int rflag = recv(*TCP_controller, command_buffer, 1+sizeof(struct node_parameters), 0);
	if (rflag == 0) return;
	if (rflag == -1){
		close(*TCP_controller);
		printf("Socket failure\n");
		exit(1);
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
		inter->usrp_tx->set_tx_rate(np->tx_rate);
		inter->tx_rate = np->tx_rate;
		inter->usrp_tx->set_tx_gain(np->tx_gain);
		inter->int_type = np->int_type;
		inter->period = np->period;
		inter->duty_cycle = np->duty_cycle;
		break;
	case 't': // terminate program
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

int main(int argc, char ** argv){
	
	float run_time;

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
	//fcntl(TCP_controller, F_SETFL, 0_NONBLOCK);

	// timing variables
	//std::clock_t total_period = inter.period*CLOCKS_PER_SEC;
	//std::clock_t transmit_period = (int)(inter.duty_cycle*(float)total_period);
	
	//printf("Total period %li\n", total_period);
	//printf("Transmit period %li\n\n", transmit_period);

    // Create Transmit Streamer Object
    uhd::stream_args_t strm_args("fc32", "sc16");
    // Can use stream_args_t to set arguments
    uhd::tx_streamer::sptr  tx_strmr = inter.usrp_tx->get_device()->get_tx_stream(strm_args);

    // Get max possible buffer size from USRP Device
    size_t max_samps = tx_strmr->get_max_num_samps();

	// buffer for transmit signal
	int buffer_len = max_samps;
	//int buffer_len = 2e3;
	std::vector<std::complex<float> > usrp_buffer_on(buffer_len);
	std::vector<std::complex<float> > usrp_buffer_off(buffer_len);

	for(int i=0; i<buffer_len; i++){
		usrp_buffer_on[i].real(1000*sin(3.1415*i/10));
		usrp_buffer_on[i].imag(1000*cos(3.1415*i/10));
		//usrp_buffer_on[i].real(1.0);
		//usrp_buffer_on[i].imag(0.0);
	}
	for(int i=0; i<buffer_len; i++){
		usrp_buffer_off[i].real(0.0);
		usrp_buffer_off[i].imag(0.0);
	}	

	int iterations = (int)(run_time/inter.period);
	int iterations_on = (int)(inter.period*inter.duty_cycle*inter.tx_rate/buffer_len);
	int iterations_off = (int)(inter.period*(1.0-inter.duty_cycle)*inter.tx_rate/buffer_len);
	/*printf("Transmit rate: %f\n", inter.tx_rate);
	printf("Transmit period: %f\n", inter.period);
	printf("Transmit duty cycle: %f\n", inter.duty_cycle);
	printf("Buffer length: %i\n", buffer_len);
	printf("Iterations on: %i\n", iterations_on);
	printf("Iterations off: %i\n\n", iterations_off);		
	*/

	// Loop
	for(int i=0; i<iterations; i++){
		//Receive_command_from_controller(&TCP_controller, &inter, &np);
		
		printf("Interferer On\n");
		//while(timer < transmit_period){
		for(int i=0; i<iterations_on; i++){
			// define signal based on interference type
			switch(inter.int_type){
			case CW:
				tx_strmr->send(
					&usrp_buffer_on.front(), usrp_buffer_on.size(),
					inter.metadata_tx
                    //TODO: Should we set a timeout here?
				);
				break;
			default:
				break;
			}

			// update timer
			//timer = clock() - start;
			//printf("Timer %li\n", timer);
		}
		//start = clock();
		//timer = 0;
		printf("Interferer Off\n");
		for(int i=0; i<iterations_off; i++){
		//while(timer < off_period){
            tx_strmr->send(
                &usrp_buffer_off.front(), usrp_buffer_off.size(),
                inter.metadata_tx
                //TODO: Should we set a timeout here?
            );
            //usleep(1e6*buffer_len/inter.tx_rate);
			// update timer
			//timer = clock() -start;
		}
	}
}











