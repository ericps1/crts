// Listen to TCP connection and respond
void * serveTCPclient(void * _sc_ptr){
	struct serveClientStruct * sc_ptr = (struct serveClientStruct*) _sc_ptr;
	struct feedbackStruct read_buffer;
	int rflag;
	// Write feedback to file
	while (1){
		rflag = recv(sc_ptr->client, &read_buffer, sizeof(struct feedbackStruct), 0);
		if (rflag == 0 || rflag == -1){
			close(sc_ptr->client);
			printf("Socket failure\n");
			exit(1);
		}
	}
	return NULL;
}

// Create a TCP socket for the server and bind it to a port
// Then sit and listen/accept all connections and write the data
// to an array that is accessible to the CE
void * startTCPServer(void * _ss_ptr)
{
	struct serverThreadStruct * ss_ptr = (struct serverThreadStruct*) _ss_ptr;
	// Local (server) address
	struct sockaddr_in servAddr;
	// Parameters of client connection
	struct sockaddr_in clientAddr; // Client address
	socklen_t client_addr_size; // Client address size
	int socket_to_client = -1;
	int reusePortOption = 1;
	pthread_t TCPServeClientThread[5]; // Threads for clients
	int client = 0; // Client counter
	// Create socket for incoming connections
	int sock_listen;
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Transmitter Failed to Create Server Socket.\n");
		exit(EXIT_FAILURE);
	}
	// Allow reuse of a port. See http://stackoverflow.com/questions/14388706/socket-options-so-reuseaddr-and-so-reuseport-how-do-they-differ-do-they-mean-t
	if (setsockopt(sock_listen, SOL_SOCKET, SO_REUSEPORT, (void*)&reusePortOption, sizeof(reusePortOption)) < 0)
	{
		fprintf(stderr, " setsockopt() failed\n");
		exit(EXIT_FAILURE);
	}
	// Construct local (server) address structure
	memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
	servAddr.sin_family = AF_INET; // Internet address family
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	servAddr.sin_port = htons(ss_ptr->serverPort); // Local port
	// Bind to the local address to a port
	if (bind(sock_listen, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
	{
		fprintf(stderr, "ERROR: bind() error\n");
		exit(EXIT_FAILURE);
	}
	// Listen and accept connections indefinitely
	while (1)
	{
		// listen for connections on socket
		if (listen(sock_listen, MAXPENDING) < 0)
		{
			fprintf(stderr, "ERROR: Failed to Set Sleeping (listening) Mode\n");
			exit(EXIT_FAILURE);
		}
		// Accept a connection from client
		socket_to_client = accept(sock_listen, (struct sockaddr *)&clientAddr, &client_addr_size);
		if (socket_to_client< 0)
		{
			fprintf(stderr, "ERROR: Sever Failed to Connect to Client\n");
			exit(EXIT_FAILURE);
		}
		// Create separate thread for each client as they are accepted.
		else {
			struct serveClientStruct sc = CreateServeClientStruct();
			sc.client = socket_to_client;
			sc.fb_ptr = ss_ptr->fb_ptr;
			sc.OTA = ss_ptr->OTA;
			sc.usingUSRPs = ss_ptr->usingUSRPs;
			pthread_create(&TCPServeClientThread[client], NULL, serveTCPclient, (void*)&sc);
			client++;
			*ss_ptr->client_ptr = socket_to_client;
		}
	}// End While loop
} // End startTCPServer()


int main(){
	// Create TCP client to controller
	unsigned int controller_port = 4444;
	char * controller_ipaddr = (char*) "192.168.1.1";
	const int TCP_controller = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_to_server < 0)
	{
		fprintf(stderr, "ERROR: Receiver Failed to Create Client Socket. \nerror: %s\n", strerror(errno));
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

	// Loop
	// Either reach end of scenario and tell controller or receive end of scenario message from controller
	while (true){
		// Read command/scenario info from controller

		// These values will have been read in from scenario
		unsigned int AP_port = 4444;
		char * AP_ipaddr = (char*) "10.0.0.2";

		// Start TCP server
		int client;
		struct serverThreadStruct ss_slave = CreateServerStruct();
		ss_slave.serverPort = serverPort;
		ss_slave.fb_ptr = &fb;
		ss_slave.client_ptr = &client;
		pthread_create(&TCPServerThread, NULL, startTCPServer, (void*)&ss_slave);

		// Set initial CR conditions (these values will have been read in or defaulted)
		CR.set_tx_freq(462.0e6f);
		CR.set_tx_rate(500e3);
		CR.et_tx_gain_soft(-12.0f);
		CR.set_tx_gain_uhd(40.0f);

		CR.set_rx_freq(462.0e6f);
		CR.set_rx_rate(500e3);
		CR.set_rx_gain_uhd(20.0f);

		// Start CR
		CR.start_rx();
		CR.start_tx();
	}
}