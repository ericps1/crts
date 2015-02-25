#include<stdlib.h>
#include<stdio.h>
#include<net/if.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<pthread.h>
#include<string>
#include<time.h>
#include<uhd/utils/msg.hpp>
#include"CR.hpp"
#include"node_parameters.hpp"
#include"read_configs.hpp"
#include<iostream>
#include<fstream>

void Receive_command_from_controller(int *TCP_controller, CognitiveRadio *CR, struct node_parameters *np){
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

		// set cognitive radio parameters
		//CR->set_ip(np->CRTS_IP);
		CR->print_metrics_flag = np->print_metrics;
		CR->log_metrics_flag = np->log_metrics;
		strcpy(CR->log_file, np->log_file);
		CR->set_tx_freq(np->tx_freq);
		CR->set_rx_freq(np->rx_freq);
		CR->set_tx_rate(np->tx_rate);
		CR->set_rx_rate(np->rx_rate);
		CR->set_tx_gain_soft(np->tx_gain_soft);
		CR->set_tx_gain_uhd(np->tx_gain);
		CR->set_rx_gain_uhd(np->rx_gain);
		CR->max_gain_tx = np->tx_max_gain;
		CR->max_gain_rx = np->rx_max_gain;
		CR->PHY_metrics = true;
		CR->set_ce(np->CE);		
		// open log file to delete any current contents
		if (CR->log_metrics_flag){
			//FILE * file;
            std::ofstream log_file;
			char log_file_name[50];
			strcpy(log_file_name, "./logs/");
			strcat(log_file_name, CR->log_file);
			//file = fopen(log_file_name, "w");
            // Open file for writing and clear contents
            log_file.open("file_name", std::ofstream::out | std::ofstream::trunc);
            if (log_file.is_open())
            {
                //fclose(file);
                log_file.close();
            }
            else
            {
                std::cout<<"Error opening log file:"<<log_file_name<<std::endl;
            }
		}
		break;
	case 't': // terminate program
		exit(1);
	}
	
}

void uhd_quiet(uhd::msg::type_t type, const std::string &msg){}

void help_CRTS_UE() {
    printf("CRTS_UE -- Start a cognitive radio UE node. Only needs to be run explicitly when using CRTS_controller with -m option.\n");
    printf(" -h : Help.\n");
    printf(" -t : Run Time - Length of time this node will run. In seconds.\n");
    printf("      Default: 20.0 s\n");
    printf(" -a : IP Address of node running CRTS_controller.\n");
}

int main(int argc, char ** argv){
	
	float run_time = 20.0f;
	float us_sleep = 5e5;
	int iterations;

    // Default IP address of controller
	char * controller_ipaddr = (char*) "192.168.1.56";

	int d;
	while((d = getopt(argc, argv, "ht:a:")) != EOF){
		switch(d){
		case 'h': help_CRTS_UE();               return 0;
		case 't': run_time = atof(optarg);      break;
		case 'a': controller_ipaddr = optarg;   break;
		}
	}

	iterations = (int) (run_time/(us_sleep*1e-6));
	printf("Iterations %i\n", iterations);
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
	
	// Quiet UHD output and fix buffer issue
	uhd::msg::register_handler(&uhd_quiet);
	
	// Create CR object
	CognitiveRadio CR;
	
	// Create node parameters struct
	struct node_parameters np;
		/*np.type = 0;
		strcpy(np.CORNET_IP, "192.168.1.25");
		strcpy(np.CRTS_IP, "10.0.0.2");
		strcpy(np.CE, "CE_UE");
		np.layers = 0;
		np.traffic = 0;
		np.freq_tx = 460e6;
		np.freq_rx = 440e6;
		np.tx_rate = 1e6;
		np.rx_rate = 1e6;
		np.gain_tx_soft = 1.0;
		np.gain_tx = 20.0;
		np.gain_rx = 20.0;
		np.max_gain_tx = 30.0;
		np.max_gain_rx = 30.0;
		np.int_type = 0;
		np.duty_cycle = 1.0;
	
		// set cognitive radio parameters
		CR.set_tx_freq(np.freq_tx);
		CR.set_rx_freq(460e6);
		CR.set_tx_rate(np.tx_rate);
		CR.set_rx_rate(np.rx_rate);
		CR.set_tx_gain_soft(np.gain_tx_soft);
		CR.set_tx_gain_uhd(np.gain_tx);
		CR.set_rx_gain_uhd(20.0);
		CR.max_gain_tx = np.max_gain_tx;
		CR.max_gain_rx = np.max_gain_rx;
		CR.PHY_metrics = true;
		*/
	// Read initial scenario info from controller (block program until data is received)
	//sleep(1.0);	
	Receive_command_from_controller(&TCP_controller, &CR, &np);
	CR.start_rx();
    //printf("Setting socket to non-blocking\n");
	//fcntl(TCP_controller, F_SETFL, O_NONBLOCK); // Set socket to non-blocking for future communication

	// Start CR
	//CR.start_tx();

	// Create dumby frame to be transmitted
	unsigned char header[8] = {};
	unsigned int payload_len = 256;
	unsigned char payload[payload_len];
	for(unsigned int i=0; i<payload_len; i++) payload[i] = i;

	// Loop
	for(int i=0; i<iterations; i++){
		// Listen for any updates from the controller (non-blocking)
		//Receive_command_from_controller(&TCP_controller, &CR, &np);

		// Create TCP client to AP
		/*const int TCP_AP = socket(AF_INET, SOCK_STREAM, 0);
		if (TCP_AP < 0)
		{
			printf("ERROR: Receiver Failed to Create Client Socket\n");
			exit(EXIT_FAILURE);
		}
		// Parameters for connecting to server
		struct sockaddr_in AP_addr;
		memset(&AP_addr, 0, sizeof(AP_addr));
		AP_addr.sin_family = AF_INET;
		AP_addr.sin_port = htons(AP_port);
		AP_addr.sin_addr.s_addr = inet_addr(AP_ipaddr);
		// Attempt to connect client socket to server
		int connect_status = connect(TCP_AP, (struct sockaddr*)&AP_addr, sizeof(AP_addr));
		if (connect_status){
			printf("Failed to Connect to server.\n");
			exit(EXIT_FAILURE);
		}*/

		// Wait (used for test purposes only)
		usleep(us_sleep);

		// Generate data according to traffic parameter
		// for now there's no difference
		switch (np.traffic){
		case burst:
			CR.transmit_packet(header, payload, payload_len);
		case stream:
			CR.transmit_packet(header, payload, payload_len);
		}
		
		// Send data via CR
		/*int nwrite = write(TCP_AP, (void*)buffer, nbytes);
		if (nwrite != nbytes){
			printf("Failed to Connect to server.\n");
			exit(EXIT_FAILURE);
		}*/

		// Either reach end of scenario and tell controller or receive end of scenario message from controller
	}
}
