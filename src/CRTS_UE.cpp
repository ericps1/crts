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
#include"CR.hpp"
#include"node_parameters.hpp"

void Receive_command_from_controller(int *TCP_controller, CognitiveRadio *CR, struct node_parameters *np){
	printf("Listening for message from server\n");
	// Listen to socket for message from controller
	char command_buffer[1+sizeof(struct node_parameters)];
	int rflag = recv(*TCP_controller, command_buffer, 1+sizeof(struct node_parameters), 0);
	printf("receive flag %i\n", rflag);
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
		memcpy(np ,&command_buffer[1], 80);

		// set cognitive radio parameters
		CR->set_tx_freq(np->freq_tx);
		CR->set_rx_freq(np->freq_rx);
		CR->set_tx_rate(np->tx_rate);
		CR->set_rx_rate(np->rx_rate);
		CR->set_tx_gain_soft(np->gain_tx_soft);
		CR->set_tx_gain_uhd(np->gain_tx);
		CR->set_rx_gain_uhd(np->gain_rx);
		CR->max_gain_tx = np->max_gain_tx;
		CR->max_gain_rx = np->max_gain_rx;
		break;
	case 't': // terminate program
		exit(1);
	}
}


int main(){
	// Create TCP client to controller
	//unsigned int controller_port = 4444;
	char * controller_ipaddr = (char*) "192.168.1.23";
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
	// Create CR object
	CognitiveRadio CR;
	printf("Cognitve Radio created\n");
	// Create node parameters struct
	struct node_parameters np;

	//char command_buffer[81];
	//int rflag = recv(TCP_controller, &command_buffer[0], 81, 0);
	//printf("receive flag %i\n", rflag);
	
	// Read initial scenario info from controller (block program until data is received)
	Receive_command_from_controller(&TCP_controller, &CR, &np);
	fcntl(TCP_controller, F_SETFL, O_NONBLOCK); // Set socket to non-blocking for future communication

	// Start CR
	sleep(2.0);
	CR.start_rx();
	//CR.start_tx();

	// Create dumby frame to be transmitted
	unsigned char header[8] = {};
	unsigned int payload_len = 256;
	unsigned char payload[payload_len];
	for(unsigned int i=0; i<payload_len; i++) payload[i] = i;

	// Loop
	while (true){
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
		sleep(2.0f);

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
