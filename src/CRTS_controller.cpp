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
#include"read_configs.hpp"

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
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
		printf("setsockopt() failed\n");
		exit(1);
	}
	// Construct local (server) address structure
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr)); // Zero out structure
	serv_addr.sin_family = AF_INET; // Internet address family
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	serv_addr.sin_port = htons(4444); // Local port
	// Bind to the local address to a port
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		printf("ERROR: bind() error\n");
		exit(1);
	}	

	// objects needs for TCP links to cognitive radio nodes
	int client[48];
	struct sockaddr_in clientAddr[48];
	socklen_t client_addr_size[48];
	struct node_parameters np[48];

	// read master scenario config file
	char scenario_list[30][60];
	int num_scenarios = read_scenario_master_file(scenario_list);
	printf("Number of scenarios: %i\n\n", num_scenarios);

	// loop through scenarios
	for (int i = 0; i < num_scenarios; i++){
		printf("Scenario %i:\n", i+1);
		printf("Config file: %s\n", &scenario_list[i][0]);
		// read the number of nodes in scenario	
		int num_nodes = read_num_nodes(&scenario_list[i][0]);
		printf("Number of nodes: %i\n", num_nodes);

		// loop through nodes in scenario
		for (int j = 0; j < num_nodes; j++){

			// read in node settings
			memset(&np[j], 0, sizeof(struct node_parameters));
			printf("Reading node %i's parameters...\n", j+1);
			np[j] = read_node_parameters(j+1, &scenario_list[i][0]);
			print_node_parameters(&np[j]);
			
			// launch appropriate executable on node
			/*switch node_settings.type{
			case AP: 
				strcpy(cmd, "./CRTS_AP");
				break;
			case UE:
				strcpy(cmd, "./CRTS_UE");
				break;
			case interferer:
				strcpy(cmd, "./CRTS_interferer");
				break;
			}*/
			
			// send command to launch executable
			char command[100] = "sshpass -p zv8p8vfa ssh ericps1@"; 
			strcat(command, np[j].CORNET_IP);
			strcat(command, " './crts/CRTS_UE >&- 2>&- <&- &'");
			//system(command);
			
			// listen for node to connect to server
			if (listen(sockfd, MAXPENDING) < 0)
			{
				fprintf(stderr, "ERROR: Failed to Set Sleeping (listening) Mode\n");
				exit(EXIT_FAILURE);
			}	
			// accept connection
			client[j] = accept(sockfd, (struct sockaddr *)&clientAddr[j], &client_addr_size[j]);
			if (client[j] < 0)
			{
				fprintf(stderr, "ERROR: Sever Failed to Connect to Client\n");
				exit(EXIT_FAILURE);
			}
			// send node parameters
			printf("\nNode %i has connected. Sending its parameters...\n", j+1);
			char msg_type = 's';
			send(client[j], (void*)&msg_type, sizeof(char), 0);
			send(client[j], (void*)&np[j], sizeof(struct node_parameters), 0);
		}

		printf("Waiting to for scenario termination message from a node\n");
		while(true){;}
	
		// tell nodes to terminate the scenario

		for(int i=0; i<num_nodes; i++){
			close(client[i]);
		}
		
		
		// Generate/push transmit data if needed
		// Receive feedback if needed
		// Determine when scenario is over either from feedback or from a message from a CR node
		// Terminate scenario on all nodes
	}
}



















