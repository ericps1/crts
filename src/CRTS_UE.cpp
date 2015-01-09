#include<stdlib.h>
#include<stdio.h>
#include<net/if.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include <pthread.h>
#include "CR.hpp"

void Receive_command_from_controller(int *TCP_controller, CognitiveRadio *CR){
	// Listen to socket for message from controller
	char command_buffer[1000];
	int rflag = recv(*TCP_controller, &command_buffer, sizeof(command_buffer), 0);
	if (rflag == 0) return;
	if (rflag == -1){
		close(*TCP_controller);
		printf("Socket failure\n");
		exit(1);
	}

	// Parse command
	// if scenario has ended, exit
}


int main(){
	// Create TCP client to controller
	unsigned int controller_port = 4444;
	char * controller_ipaddr = (char*) "192.168.1.1";
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
	controller_addr.sin_port = htons(controller_port);
	controller_addr.sin_addr.s_addr = inet_addr(controller_ipaddr);

	// Attempt to connect client socket to server
	int connect_status = connect(TCP_controller, (struct sockaddr*)&controller_addr, sizeof(controller_addr));
	if (connect_status){
		printf("Failed to Connect to server.\n");
		exit(EXIT_FAILURE);
	}

	// Create CR object
	CognitiveRadio CR;

	// Read initial scenario info from controller (block program until data is received)
	Receive_command_from_controller(&TCP_controller, &CR);
	fcntl(TCP_controller, F_SETFL, O_NONBLOCK); // Set socket to non-blocking for future communication

	// Loop
	while (true){
		// Listen for any updates from the controller (non-blocking)
		Receive_command_from_controller(&TCP_controller, &CR);

		// These values will have been read in from scenario
		unsigned int AP_port = 4444;
		char * AP_ipaddr = (char*) "10.0.0.2";

		// Create TCP client to AP
		const int TCP_AP = socket(AF_INET, SOCK_STREAM, 0);
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
		}

		// Set initial CR conditions (these values will have been read in or defaulted)
		CR.set_tx_freq(462.0e6f);
		CR.set_tx_rate(500e3);
		CR.set_tx_gain_soft(-12.0f);
		CR.set_tx_gain_uhd(40.0f);

		CR.set_rx_freq(462.0e6f);
		CR.set_rx_rate(500e3);
		CR.set_rx_gain_uhd(20.0f);

		// Start CR
		CR.start_rx();
		CR.start_tx();

		// Either read data from controller or generate data (continuous of probabilistically)
		int nbytes = 1024;
		char buffer[nbytes];
		for (int i = 0; i < nbytes; i++) buffer[i] = (int) (255.0*(float)rand() / (float)RAND_MAX);
		
		// Send data via CR
		int nwrite = write(TCP_AP, (void*)buffer, nbytes);
		if (nwrite != nbytes){
			printf("Failed to Connect to server.\n");
			exit(EXIT_FAILURE);
		}

		// Either reach end of scenario and tell controller or receive end of scenario message from controller
	}
}
