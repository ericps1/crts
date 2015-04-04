#include <stdlib.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <string>
#include <time.h>
#include <uhd/utils/msg.hpp>
#include<iostream>
#include<fstream>
#include <errno.h>
#include <signal.h>
#include "ECR.hpp"
#include "node_parameters.hpp"
#include "read_configs.hpp"
#include "TUN.hpp"

#define DEBUG 0
#if DEBUG == 1
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

int sig_terminate;

void Receive_command_from_controller(int *TCP_controller, ExtensibleCognitiveRadio *ECR, struct node_parameters *np){
	// Listen to socket for message from controller
	char command_buffer[500+sizeof(struct node_parameters)];
	int rflag = recv(*TCP_controller, command_buffer, 1+sizeof(struct node_parameters), 0);
	int err = errno;
	if(rflag <= 0){
		if ((err == EAGAIN) || (err == EWOULDBLOCK))
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

		// set cognitive radio parameters
		ECR->set_ip(np->CRTS_IP);
		ECR->print_metrics_flag = np->print_metrics;
		ECR->log_metrics_flag = np->log_metrics;
		ECR->set_ce_timeout_ms(np->ce_timeout_ms);
		strcpy(ECR->log_file, np->log_file);
		ECR->set_tx_freq(np->tx_freq);
		ECR->set_rx_freq(np->rx_freq);
		ECR->set_tx_rate(np->tx_rate);
		ECR->set_rx_rate(np->rx_rate);
		ECR->set_tx_gain_soft(np->tx_gain_soft);
		ECR->set_tx_gain_uhd(np->tx_gain);
		ECR->set_rx_gain_uhd(np->rx_gain);
		ECR->set_ce(np->CE);		

		// open log file to delete any current contents
		if (ECR->log_metrics_flag){
			//FILE * file;
            std::ofstream log_file;
			char log_file_name[50];
			strcpy(log_file_name, "./logs/");
			strcat(log_file_name, ECR->log_file);
			//file = fopen(log_file_name, "w");
            // Open file for writing and clear contents
            log_file.open(log_file_name, std::ofstream::out | std::ofstream::trunc);
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
		printf("Received termination command from controller\n");
		exit(1);
	}
}

void uhd_quiet(uhd::msg::type_t type, const std::string &msg){}

void help_ECRTS_UE() {
    printf("ECRTS_UE -- Start a cognitive radio UE node. Only needs to be run explicitly when using ECRTS_controller with -m option.\n");
    printf("        -- This program must be run from the main ECRTS directory.\n");
    printf(" -h : Help.\n");
    printf(" -t : Run Time - Length of time this node will run. In seconds.\n");
    printf("      Default: 20.0 s\n");
    printf(" -a : IP Address of node running ECRTS_controller.\n");
}

void terminate(int signum){
	printf("\nSending termination message to controller\n");
	sig_terminate = 1;
}

int main(int argc, char ** argv){
	
	// register signal handlers
	signal(SIGINT, terminate);
	signal(SIGQUIT, terminate);
	signal(SIGTERM, terminate);
	
	// timing variables
	float run_time = 20.0f;
	float us_sleep = 1e6;
	int iterations;

    // Default IP address of controller
	char * controller_ipaddr = (char*) "192.168.1.56";

	int d;
	while((d = getopt(argc, argv, "ht:a:")) != EOF){
		switch(d){
		case 'h': help_ECRTS_UE();               return 0;
		case 't': run_time = atof(optarg);      break;
		case 'a': controller_ipaddr = optarg;   break;
		}
	}

	// Set some number of iterations based on the run time and delay between iterations
	iterations = (int) (run_time/(us_sleep*1e-6));
	
	// Create TCP client to controller
	unsigned int controller_port = 4444;
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
	controller_addr.sin_port = htons(controller_port);
	
	// Attempt to connect client socket to server
	int connect_status = connect(TCP_controller, (struct sockaddr*)&controller_addr, sizeof(controller_addr));
	if (connect_status){
		printf("Failed to Connect to server.\n");
		exit(EXIT_FAILURE);
	}
	dprintf("Connected to server\n");
	
	// Quiet UHD output
	uhd::msg::register_handler(&uhd_quiet);
	
	// Create ECR object
	dprintf("Creating ECR object...\n");
	ExtensibleCognitiveRadio ECR;
	
	// Create node parameters struct and read scenario info from controller
	struct node_parameters np;
	dprintf("Receiving command from controller...\n");
	Receive_command_from_controller(&TCP_controller, &ECR, &np);
	fcntl(TCP_controller, F_SETFL, O_NONBLOCK); // Set socket to non-blocking for future communication

	// Start ECR
	dprintf("Starting ECR object...\n");
	ECR.start_rx();
    ECR.start_tx();

	// Port to be used by CRTS server and client
	int port = 4444;

	// Define address structure for CRTS socket server used to receive network traffic
	struct sockaddr_in CRTS_server_addr;
	memset(&CRTS_server_addr, 0, sizeof(CRTS_server_addr));
	CRTS_server_addr.sin_family = AF_INET;
	CRTS_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	CRTS_server_addr.sin_port = htons(port);
	socklen_t clientlen = sizeof(CRTS_server_addr);
	int CRTS_server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Define address structure for CRTS socket client used to send network traffic
	struct sockaddr_in CRTS_client_addr;
	memset(&CRTS_client_addr, 0, sizeof(CRTS_client_addr));
	CRTS_client_addr.sin_family = AF_INET;
	CRTS_client_addr.sin_addr.s_addr = inet_addr(np.TARGET_IP);
	CRTS_client_addr.sin_port = htons(port);
	socklen_t serverlen = sizeof(CRTS_client_addr);
	int CRTS_client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Bind CRTS server socket
	bind(CRTS_server_sock, (sockaddr*)&CRTS_server_addr, clientlen);

	// set CRTS sockets to non-blocking
	fcntl(CRTS_client_sock, F_SETFL, O_NONBLOCK);
	fcntl(CRTS_server_sock, F_SETFL, O_NONBLOCK);
	
	// Define a buffer for receiving and a temporary message for sending
	int recv_buffer_len = 8192*2;
	char recv_buffer[recv_buffer_len];
	char message[40]; 
	strcpy(message, "Test Message from "); 
	strcat(message, np.CRTS_IP);	
	
	// initialize sig_terminate flag and check return from socket call
	sig_terminate = 0;
	if(CRTS_client_sock<0){
		printf("CRTS failed to create client socket\n");
		sig_terminate = 1;
	}
	if(CRTS_server_sock<0){
		printf("CRTS failed to create server socket\n");
		sig_terminate = 1;
	}

	// main loop
	for(int i=0; i<iterations; i++){
		// Listen for any updates from the controller (non-blocking)
		dprintf("Listening to controller for command\n");
		Receive_command_from_controller(&TCP_controller, &ECR, &np);
		
		// Wait (used for test purposes only)
		usleep(us_sleep);

		// if not using FDD then stop the receiver before transmitting
		if(np.duplex != FDD){ 
			ECR.stop_rx();
			usleep(1e3);
		}
		// Generate data according to traffic parameter
		// Burst is not yet implemented
		switch (np.traffic){
		case burst:
			// TODO generate data according to some probabilistic model (probably simple Poisson process)
			break;
		case stream:
			dprintf("Sending UDP packet using CRTS client socket\n");
			int send_return = sendto(CRTS_client_sock, message, strlen(message), 0, (struct sockaddr*)&CRTS_client_addr, sizeof(CRTS_client_addr));	
			if(send_return < 0) printf("Failed to send message\n");
			break;
		}
		// restart receiver if it was stopped earlier
		if(np.duplex != FDD) ECR.start_rx();
			
		// read all available data from the UDP socket
		int recv_len =0;
		dprintf("CRTS: Reading from CRTS server socket\n");
		recv_len = recvfrom(CRTS_server_sock, recv_buffer, recv_buffer_len, 0, (struct sockaddr *)&CRTS_server_addr, &clientlen);
		// print out received messages
		if(recv_len > 0){
			printf("\nCRTS_UE received message:\n");
			for(int j=0; j<recv_len; j++)
				printf("%c", recv_buffer[j]);
			printf("\n");
		}
				
		// Either reach end of scenario and tell controller or receive end of scenario message from controller
		if(sig_terminate) break;
		if(i == iterations-1) printf("Run time has been reached\n");
	}

	printf("Sending termination message to controller\n");
	char term_message = 't';
	write(TCP_controller, &term_message, 1);
}

