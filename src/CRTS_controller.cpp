int main(){
	// read master scenario config
	readScMasterFile(char *scenario_list[30], int verbose);

	// create TCP server
	int reusePortOption = 1;
	int client = 0; // Client counter
	// Create socket for incoming connections
	int sock_listen;
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Transmitter Failed to Create Server Socket.\n");
		exit(EXIT_FAILURE);
	}
	// Allow reuse of a port. See http://stackoverflow.com/questions/14388706/socket-options-so-reuseaddr-and-so-reuseport-how-do-they-differ-do-they-mean-t
	if (setsockopt(sock_listen, SOL_SOCKET, SO_REUSEPORT, (void*)&reusePortOption, sizeof(reusePortOption)) < 0){
		printf(" setsockopt() failed\n");
		exit(EXIT_FAILURE);
	}
	// Construct local (server) address structure
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
	servAddr.sin_family = AF_INET; // Internet address family
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	servAddr.sin_port = htons("4444"); // Local port
	// Bind to the local address to a port
	if (bind(sock_listen, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0){
		printf("ERROR: bind() error\n");
		exit(EXIT_FAILURE);
	}

	// struct for node settings (maximum of 15 nodes)
	struct node_settings_s node_settings[15] = {};

	// loop through scenarios
	for (int i = 0; i < num_scenarios; i++){
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

			// listen for node to connect to server
			if (listen(sock_listen, MAXPENDING) < 0)
			{
				fprintf(stderr, "ERROR: Failed to Set Sleeping (listening) Mode\n");
				exit(EXIT_FAILURE);
			}
			// Accept a connection from client
			struct sockaddr_in clientAddr; // Client address
			socklen_t client_addr_size; // Client address size
			int socket_to_client = -1;
			socket_to_client = accept(sock_listen, (struct sockaddr *)&clientAddr, &client_addr_size);
			if (socket_to_client < 0)
			{
				fprintf(stderr, "ERROR: Sever Failed to Connect to Client\n");
				exit(EXIT_FAILURE);
			}
			
			write(client, (void*)&node_settings, sizeof(struct node_settings_s));
		}

		// Generate/push transmit data if needed
		// Receive feedback if needed
		// Determine when scenario is over either from feedback or from a message from a CR node
		// Terminate scenario on all nodes
	}
}