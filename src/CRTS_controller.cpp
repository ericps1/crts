#include<stdlib.h>
#include<stdio.h>
#include<net/if.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>
#include<ctype.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<pthread.h>
#include<string.h>
#include<time.h>
#include"node_parameters.hpp"

#define MAXPENDING 5

int main(){
	// create TCP server
	//int reusePortOption = 1;
	//int client_count = 0; // Client counter
	// Create socket for incoming connections
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Transmitter Failed to Create Server Socket.\n");
		exit(EXIT_FAILURE);
	}
	// Allow reuse of a port. See http://stackoverflow.com/questions/14388706/socket-options-so-reuseaddr-and-so-reuseport-how-do-they-differ-do-they-mean-t
	/*if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (void*)&reusePortOption, sizeof(reusePortOption)) < 0){
		printf(" setsockopt() failed\n");
		exit(EXIT_FAILURE);
	}*/
	// Construct local (server) address structure
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr)); // Zero out structure
	serv_addr.sin_family = AF_INET; // Internet address family
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	serv_addr.sin_port = htons(4444); // Local port
	// Bind to the local address to a port
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		printf("ERROR: bind() error\n");
		exit(EXIT_FAILURE);
	}	

	// node settings
	struct node_parameters np1 = {};
		np1.freq_tx = 400e6;
		np1.freq_rx = 420e6;
		np1.tx_rate = 1e6;
		np1.rx_rate = 1e6;
		np1.gain_tx_soft = 1.0f;
		np1.gain_tx = 20.0;
		np1.gain_rx = 20.0;
	struct node_parameters np2 = {};
		np2.freq_tx = 420e6;
		np2.freq_rx = 400e6;
		np2.tx_rate = 1e6;
		np2.rx_rate = 1e6;
		np2.gain_tx_soft = 1.0f;
		np2.gain_tx = 20.0;
		np2.gain_rx = 20.0;

	// loop through scenarios
	/*for (int i = 0; i < num_scenarios; i++){
		// read the number of nodes in scenario
		read_num_nodes(scenario_list[i]);

		// loop through nodes in scenario
		for (int j = 0; j < num_nodes; j++){

			// read in node settings
			node_settings[j] = read_node(j, scenario_list[i]);

			// launch appropriate executable on node
			char *cmd;
			switch node_settings.type{
			case AP: 
				strcpy(cmd, "./CRTS_AP");
				break;
			case UE:
				strcpy(cmd, "./CRTS_UE");
				break;
			case interferer:
				strcpy(cmd, "./CRTS_interferer");
				break;
			}
			launch_remote_exe(node_settings[j].CORNET_IP, cmd);
	*/
			// listen for node to connect to server
			
			// send info to first connected node
			if (listen(sockfd, MAXPENDING) < 0)
			{
				fprintf(stderr, "ERROR: Failed to Set Sleeping (listening) Mode\n");
				exit(EXIT_FAILURE);
			}	
			// Accept a connection from client
			struct sockaddr_in clientAddr1; // Client address
			socklen_t client_addr_size; // Client address size
			int client1 = -1;
			client1 = accept(sockfd, (struct sockaddr *)&clientAddr1, &client_addr_size);
			if (client1 < 0)
			{
				fprintf(stderr, "ERROR: Sever Failed to Connect to Client\n");
				exit(EXIT_FAILURE);
			}
			printf("Sending scenario parameters to first node\n");
			char msg_type = 's';
			send(client1, (void*)&msg_type, 1, 0);
			send(client1, (void*)&np1, sizeof(struct node_parameters), 0);
			
			// send info to second connected node
			if (listen(sockfd, MAXPENDING) < 0)
			{
				fprintf(stderr, "ERROR: Failed to Set Sleeping (listening) Mode\n");
				exit(EXIT_FAILURE);
			}
			// Accept a connection from client
			struct sockaddr_in clientAddr2; // Client address
			//socklen_t client_addr_size; // Client address size
			int client2 = -1;
			client2 = accept(sockfd, (struct sockaddr *)&clientAddr2, &client_addr_size);
			if (client2 < 0)
			{
				fprintf(stderr, "ERROR: Sever Failed to Connect to Client\n");
				exit(EXIT_FAILURE);
			}
			printf("Sending scenario parameters to second node\n");
			send(client2, (void*)&msg_type, 1, 0);
			send(client2, (void*)&np2, sizeof(struct node_parameters), 0);
			
		//}

		// Generate/push transmit data if needed
		// Receive feedback if needed
		// Determine when scenario is over either from feedback or from a message from a CR node
		// Terminate scenario on all nodes
	//}
}
