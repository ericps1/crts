#include <stdlib.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "node_parameters.hpp"
#include "read_configs.hpp"

#define MAXPENDING 5

// global variables
int sig_terminate;
int num_nodes_terminated;

int Receive_msg_from_nodes(int *client, int num_nodes){
	// Listen to sockets for messages from any node
	char msg;
	for(int i=0; i<num_nodes; i++){
		int rflag = recv(client[i], &msg, 1, 0);
		int err = errno;
		if (rflag <= 0){ 
			if(!((err == EAGAIN) || (err == EWOULDBLOCK))){
				close(client[i]);
				printf("Node %i has disconnected. Terminating the current scenario..\n\n", i);
				// tell all nodes to terminate program
				for(int j=0; j<num_nodes; j++){
					write(client[j], &msg, 1);
				}
				return 1;
			}
		}	
		// Parse command if received a message
		else{
			switch (msg){
			case 't': // terminate program
				printf("Node %i has sent a termination message...\n", i);
				num_nodes_terminated++;
				// check if all nodes have terminated
				if(num_nodes_terminated == num_nodes) return 1;
				break;
			default:
				printf("Invalid message type received from node %i\n", i);
			}
		}
	}

	return 0;
}

void help_CRTS_controller() {
    printf("CRTS_controller -- Initiate cognitive radio testing.\n");
    printf(" -h : Help.\n");
    printf(" -m : Manual Mode - Start each node manually rather than have CRTS_controller do it automatically.\n");
    printf(" -u : Username - For logging into other nodes via ssh.\n");
    printf(" -l : CRTS Location - Directory containing CRTS Executables. Must be same for all remote nodes.\n");
    printf("      Default: ~/crts\n");
    printf(" -a : IP Address - IP address of this computer as seen by remote nodes.\n");
    printf("      Default: 192.168.1.56\n");
}

void terminate(int signum){
	printf("Terminating scenario on all nodes\n");
	sig_terminate = 1;	
}

int main(int argc, char ** argv){
	
	// register signal handlers
	signal(SIGINT, terminate);
	signal(SIGQUIT, terminate);
	signal(SIGTERM, terminate);
	//signal(SIGPIPE, terminate);
	
	int manual_execution = 0;

    // Default username for ssh
    char * ssh_uname = (char *) "ericps1";
    // Default location of CRTS Directory
    char * crts_dir = (char *) "~/crts/";
    // Default IP address of server as seen by other nodes
    // TODO: Autodetect IP address as seen by other nodes
    char * serv_ip_addr = (char *) "192.168.1.56";

	int d;
	while((d = getopt(argc, argv, "hmu:l:a:")) != EOF){
		switch (d){
            case 'h': help_CRTS_controller();   return 0;
            case 'm': manual_execution = 1;     break;
            case 'u': ssh_uname = optarg;       break;
            case 'l': crts_dir = optarg;        break;
            case 'a': serv_ip_addr = optarg;    break;
		}
	}
	
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
    // FIXME: May not be necessary in this version of CRTS
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
	/*int client2;
	struct sockaddr_in clientAddr2;
	socklen_t client_addr_size2;
	struct node_parameters np2;
	*/

	// read master scenario config file
	char scenario_list[30][60];
	int num_scenarios = read_scenario_master_file(scenario_list);
	printf("Number of scenarios: %i\n\n", num_scenarios);

	// loop through scenarios
	for (int i = 0; i < num_scenarios; i++){
		printf("Scenario %i:\n", i+1);
		printf("Config file: %s\n", &scenario_list[i][0]);
		// read the number of nodes in scenario	
		struct scenario_parameters sp = read_scenario_parameters(&scenario_list[i][0]);
		printf("Number of nodes: %i\n", sp.num_nodes);
		printf("Run time: %f\n", sp.run_time);

		// loop through nodes in scenario
		for (int j = 0; j < sp.num_nodes; j++){

			// read in node settings
			memset(&np[j], 0, sizeof(struct node_parameters));
			printf("Reading node %i's parameters...\n", j+1);
			np[j] = read_node_parameters(j+1, &scenario_list[i][0]);
			print_node_parameters(&np[j]);
			
			// send command to launch executable if not doing so manually
			if (!manual_execution){
				char command[100] = "ssh "; 
                // Add username to ssh command
				strcat(command, ssh_uname);
				strcat(command, "@");
				strcat(command, np[j].CORNET_IP);
				strcat(command, " 'sleep 1 && ");
				strcat(command, crts_dir);
			
				// add appropriate executable
				switch (np[j].type){
				case BS: 
					strcat(command, "/CRTS_AP");
					break;
				case UE:
					strcat(command, "/CRTS_UE");
					break;
				case interferer:
					strcat(command, "/CRTS_interferer");
					break;
				}
		
				// append run time 
				strcat(command, " -t ");
				char run_time_str[10];
				sprintf(run_time_str, "%f", sp.run_time);
				strcat(command, run_time_str);

				// append IP Address of controller
				strcat(command, " -a ");
				strcat(command, serv_ip_addr);

                // set command to continue program after shell is closed
				strcat(command, " 2>&1 &'& ");
				system(command);
				printf("Command executed: %s\n", command);
			}

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

			// set socket to non-blocking
			fcntl(client[j], F_SETFL, O_NONBLOCK);

			// send node parameters
			printf("\nNode %i has connected. Sending its parameters...\n", j+1);
			char msg_type = 's';
			send(client[j], (void*)&msg_type, sizeof(char), 0);
			send(client[j], (void*)&np[j], sizeof(struct node_parameters), 0);			
		}

		printf("Waiting to for scenario termination message from a node\n");
		sig_terminate = 0;
		int msg_terminate = 0;
		num_nodes_terminated = 0;
		while((!sig_terminate) && (!msg_terminate)){
			msg_terminate = Receive_msg_from_nodes(&client[0], sp.num_nodes);
		}
		
		// if the controller is being terminated, send termination message to other nodes
		if(sig_terminate){
			char msg = 't';
			for(int j=0; j<sp.num_nodes; j++){
				write(client[j], &msg, 1);
			}
		}

		// Generate/push transmit data if needed
		// Receive feedback if needed
		// Determine when scenario is over either from feedback or from a message from a CR node
		// Terminate scenario on all nodes
	}
}



















