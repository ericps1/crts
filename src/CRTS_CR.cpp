#include <stdlib.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <string>
#include <uhd/utils/msg.hpp>
#include <uhd/types/time_spec.hpp>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <signal.h>
#include <random>
#include "CRTS.hpp"
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
time_t stop_time_s;
std::ofstream log_rx_fstream;
std::ofstream log_tx_fstream;

void Receive_command_from_controller(int *TCP_controller,
                                     struct scenario_parameters *sp,
                                     struct node_parameters *np) {
  // Listen to socket for message from controller
  char command_buffer[1 + sizeof(struct scenario_parameters) +
                      sizeof(struct node_parameters)];
  memset(&command_buffer, 0, sizeof(command_buffer));

  // setup file descriptor to listen for data on TCP controller link.
  fd_set fds;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 100;
  FD_ZERO(&fds);
  FD_SET(*TCP_controller, &fds);

  // if data is available, read it in
  if (select(*TCP_controller + 1, &fds, NULL, NULL, &timeout)) {
    // read the first byte which designates the message type
    int rflag = recv(*TCP_controller, command_buffer, 1, 0);

    int err = errno;
    if (rflag <= 0) {
      if ((err == EAGAIN) || (err == EWOULDBLOCK))
        return;
      else {
        close(*TCP_controller);
        printf("Socket failure\n");
        sig_terminate = 1;
      }
    }

    // Parse command based on the message type
    switch (command_buffer[0]) {
    case CRTS_MSG_SCENARIO_PARAMETERS: // settings for upcoming scenario
      printf("Received settings for scenario\n");
      // receive and copy scenario parameters
      rflag = recv(*TCP_controller, &command_buffer[1],
                   sizeof(struct scenario_parameters), 0);
      memcpy(sp, &command_buffer[1], sizeof(struct scenario_parameters));

      // receive and copy node_parameters
      rflag = recv(*TCP_controller,
                   &command_buffer[1 + sizeof(struct scenario_parameters)],
                   sizeof(struct node_parameters), 0);
      printf("Received %i bytes\n", rflag);
	  memcpy(np, &command_buffer[1 + sizeof(struct scenario_parameters)],
             sizeof(struct node_parameters));
      print_node_parameters(np);
      break;
    case CRTS_MSG_MANUAL_START: // updated start time (used for manual mode)
      rflag = recv(*TCP_controller, &command_buffer[1], sizeof(time_t), 0);
      memcpy(&sp->start_time_s, &command_buffer[1], sizeof(time_t));
      stop_time_s = sp->start_time_s + sp->runTime;
      break;
    case CRTS_MSG_TERMINATE: // terminate program
      printf("Received termination command from controller\n");
      sig_terminate = 1;
    }
  }
}

void Initialize_CR(struct node_parameters *np, void *ECR_p) {

  // initialize ECR parameters if applicable
  if (np->cr_type == ecr) {
    ExtensibleCognitiveRadio *ECR = (ExtensibleCognitiveRadio *)ECR_p;

    // append relative locations for log files
    char phy_rx_log_file_name[100];
    strcpy(phy_rx_log_file_name, "./logs/bin/");
    strcat(phy_rx_log_file_name, np->phy_rx_log_file);
    strcat(phy_rx_log_file_name, ".log");

    char phy_tx_log_file_name[100];
    strcpy(phy_tx_log_file_name, "./logs/bin/");
    strcat(phy_tx_log_file_name, np->phy_tx_log_file);
    strcat(phy_tx_log_file_name, ".log");

    // set cognitive radio parameters
    ECR->set_ip(np->CRTS_IP);
    ECR->print_metrics_flag = np->print_metrics;
    ECR->log_phy_rx_flag = np->log_phy_rx;
    ECR->log_phy_tx_flag = np->log_phy_tx;
    ECR->set_ce_timeout_ms(np->ce_timeout_ms);
    strcpy(ECR->phy_rx_log_file, phy_rx_log_file_name);
    strcpy(ECR->phy_tx_log_file, phy_tx_log_file_name);
    ECR->set_rx_freq(np->rx_freq);
    ECR->set_rx_rate(np->rx_rate);
    ECR->set_rx_gain_uhd(np->rx_gain);
    ECR->set_rx_subcarriers(np->rx_subcarriers);
    ECR->set_rx_cp_len(np->rx_cp_len);
    ECR->set_rx_taper_len(np->rx_taper_len);
    ECR->set_tx_freq(np->tx_freq);
    ECR->set_tx_rate(np->tx_rate);
    ECR->set_tx_gain_soft(np->tx_gain_soft);
    ECR->set_tx_gain_uhd(np->tx_gain);
    ECR->set_tx_subcarriers(np->tx_subcarriers);
    ECR->set_tx_cp_len(np->tx_cp_len);
    ECR->set_tx_taper_len(np->tx_taper_len);
    ECR->set_tx_modulation(np->tx_modulation);
    ECR->set_tx_crc(np->tx_crc);
    ECR->set_tx_fec0(np->tx_fec0);
    ECR->set_tx_fec1(np->tx_fec1);
    ECR->set_ce(np->CE);
    ECR->reset_log_files();

    // copy subcarrier allocations if other than liquid-dsp default
    if (np->tx_subcarrier_alloc_method == CUSTOM_SUBCARRIER_ALLOC ||
        np->tx_subcarrier_alloc_method == STANDARD_SUBCARRIER_ALLOC) {
      ECR->set_tx_subcarrier_alloc(np->tx_subcarrier_alloc);
    }
    if (np->rx_subcarrier_alloc_method == CUSTOM_SUBCARRIER_ALLOC ||
        np->rx_subcarrier_alloc_method == STANDARD_SUBCARRIER_ALLOC) {
      ECR->set_rx_subcarrier_alloc(np->rx_subcarrier_alloc);
    }
  }
  // intialize python radio if applicable
  else if (np->cr_type == python) {
    // set IP for TUN interface
    // char command[50];
    // sprintf(command, "ifconfig tunCRTS %s", np->CRTS_IP);
    // system("ip link set dev tunCRTS up");
    // sprintf(command, "ip addr add %s/24 dev tunCRTS", np->CRTS_IP);
    // system(command);
    // printf("Running command: %s\n", command);
    // system("route add -net 10.0.0.0 netmask 255.255.255.0 dev tunCRTS");
    // system("ifconfig");
  }
}

void log_rx_data(struct scenario_parameters *sp, struct node_parameters *np,
                 int bytes, int packet_num) {
  // update current time
  struct timeval tv;
  gettimeofday(&tv, NULL);

  // open file, append parameters, and close
  if (log_rx_fstream.is_open()) {
    log_rx_fstream.write((char *)&tv, sizeof(tv));
    log_rx_fstream.write((char *)&bytes, sizeof(bytes));
	log_rx_fstream.write((char *)&packet_num, sizeof(packet_num));
  } else
    printf("Error opening log file: %s\n", np->net_rx_log_file);
}

void log_tx_data(struct scenario_parameters *sp, struct node_parameters *np,
                 int bytes, int packet_num) {
  // update current time
  struct timeval tv;
  gettimeofday(&tv, NULL);

  // open file, append parameters, and close
  if (log_tx_fstream.is_open()) {
    log_tx_fstream.write((char *)&tv, sizeof(tv));
    log_tx_fstream.write((char *)&bytes, sizeof(bytes));
	log_tx_fstream.write((char *)&packet_num, sizeof(packet_num));
  } else
    printf("Error opening log file: %s\n", np->net_tx_log_file);
}

void help_CRTS_CR() {
  printf("CRTS_CR -- Start a cognitive radio node. Only needs to be run "
         "explicitly when using CRTS_controller with -m option.\n");
  printf("        -- This program must be run from the main CRTS directory.\n");
  printf(" -h : Help.\n");
  printf(" -a : IP Address of node running CRTS_controller.\n");
}

void terminate(int signum) {
  printf("\nSending termination message to controller\n");
  sig_terminate = 1;
}

int main(int argc, char **argv) {

  // register signal handlers
  signal(SIGINT, terminate);
  signal(SIGQUIT, terminate);
  signal(SIGTERM, terminate);

  // timing variables
  // time_t runTime = 20;

  // Default IP address of controller
  char *controller_ipaddr = (char *)"192.168.1.56";

  int d;
  while ((d = getopt(argc, argv, "ha:")) != EOF) {
    switch (d) {
    case 'h':
      help_CRTS_CR();
      return 0;
    case 'a':
      controller_ipaddr = optarg;
      break;
    }
  }

  // Create TCP client to controller
  unsigned int controller_port = 4444;
  int TCP_controller = socket(AF_INET, SOCK_STREAM, 0);
  if (TCP_controller < 0) {
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
  int connect_status =
      connect(TCP_controller, (struct sockaddr *)&controller_addr,
              sizeof(controller_addr));
  if (connect_status) {
    printf("Failed to Connect to server.\n");
    exit(EXIT_FAILURE);
  }
  dprintf("Connected to server\n");

  // Port to be used by CRTS server and client
  int port = 4444;

  // Create node parameters struct and the scenario parameters struct
  // and read info from controller
  struct node_parameters np;
  memset(&np, 0, sizeof(np));
  struct scenario_parameters sp;
  dprintf("Receiving command from controller...\n");
  sleep(1);
  Receive_command_from_controller(&TCP_controller, &sp, &np);
  // fcntl(TCP_controller, F_SETFL, O_NONBLOCK); // Set socket to non-blocking
  // for future communication

  // copy log file name for post processing later
  char net_rx_log_file_cpy[100];
  strcpy(net_rx_log_file_cpy, np.net_rx_log_file);
  char net_tx_log_file_cpy[100];
  strcpy(net_tx_log_file_cpy, np.net_tx_log_file);
  
  // modify log file name in node parameters for logging function
  char net_rx_log_file[100];
  strcpy(net_rx_log_file, "./logs/bin/");
  strcat(net_rx_log_file, np.net_rx_log_file);
  strcat(net_rx_log_file, ".log");
  strcpy(np.net_rx_log_file, net_rx_log_file);

  char net_tx_log_file[100];
  strcpy(net_tx_log_file, "./logs/bin/");
  strcat(net_tx_log_file, np.net_tx_log_file);
  strcat(net_tx_log_file, ".log");
  strcpy(np.net_tx_log_file, net_tx_log_file);

  // open CRTS log files to delete any current contents
  if (np.log_net_rx) {
    log_rx_fstream.open(net_rx_log_file,
                     std::ofstream::out | std::ofstream::trunc);
    if (!log_rx_fstream.is_open()) {
      std::cout << "Error opening log file:" << net_rx_log_file << std::endl;
    }
  }
  if (np.log_net_tx) {
    log_tx_fstream.open(net_tx_log_file,
                     std::ofstream::out | std::ofstream::trunc);
    if (!log_tx_fstream.is_open()) {
      std::cout << "Error opening log file:" << net_rx_log_file << std::endl;
    }
  }

  // this is used to create a child process for python radios which can be
  // killed later
  pid_t pid = 0;

  // pointer to ECR which may or may not be used
  ExtensibleCognitiveRadio *ECR = NULL;

  // Create and start the ECR or python CR so that they are in a ready
  // state when the experiment begins
  if (np.cr_type == ecr) {
    dprintf("Creating ECR object...\n");
    ECR = new ExtensibleCognitiveRadio;

    // set the USRP's timer to 0
    uhd::time_spec_t t0(0, 0, 1e6);
    ECR->usrp_rx->set_time_now(t0, 0);

    Initialize_CR(&np, (void *)ECR);

  } else if (np.cr_type == python) {
    dprintf("CRTS: Forking child process\n");
    pid_t pid = fork();

    // define child's process
    if (pid == 0) {
      char command[2000] = "sudo python cognitive_radios/";
      strcat(command, np.python_file);
      for (int i = 0; i < np.num_arguments; i++) {
        strcat(command, " ");
        strcat(command, np.arguments[i]);
      }
      strcat(command, " &");
      int ret_value = system(command);
      if (ret_value != 0)
        std::cout << "error starting python radio" << std::endl;

      sleep(5);
      dprintf("CRTS Child: Initializing python CR\n");
      Initialize_CR(&np, NULL);

      while (true) {
        if (sig_terminate)
          break;
      };

      dprintf("CRTS Child: Closing sockets\n");
      close(TCP_controller);
      exit(1);
    }
  }

  // Define address structure for CRTS socket server used to receive network
  // traffic
  struct sockaddr_in CRTS_server_addr;
  memset(&CRTS_server_addr, 0, sizeof(CRTS_server_addr));
  CRTS_server_addr.sin_family = AF_INET;
  // Only receive packets addressed to the CRTS_IP
  CRTS_server_addr.sin_addr.s_addr = inet_addr(np.CRTS_IP);
  CRTS_server_addr.sin_port = htons(port);
  socklen_t clientlen = sizeof(CRTS_server_addr);
  int CRTS_server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // Define address structure for CRTS socket client used to send network
  // traffic
  struct sockaddr_in CRTS_client_addr;
  memset(&CRTS_client_addr, 0, sizeof(CRTS_client_addr));
  CRTS_client_addr.sin_family = AF_INET;
  CRTS_client_addr.sin_addr.s_addr = inet_addr(np.TARGET_IP);
  CRTS_client_addr.sin_port = htons(port);
  int CRTS_client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // Resize send buffer
  /*int size;
  socklen_t len;
  size = 288*25;
  buff_err = setsockopt(CRTS_client_sock, SOL_SOCKET, SO_SNDBUF, &size, sizeof(int));
  */

  // Bind CRTS server socket
  bind(CRTS_server_sock, (sockaddr *)&CRTS_server_addr, clientlen);

  // Define a buffer for receiving and a temporary message for sending
  int recv_buffer_len = 8192 * 2;
  char recv_buffer[recv_buffer_len];
  
  // Define parameters and message for sending
  int packet_counter = 0;
  const int packet_num_bytes = 4; // number of bytes used for the packet number
  unsigned char packet_num_prs[packet_num_bytes]; // pseudo-random sequence used to modify packet number
  unsigned char message[CRTS_CR_NET_PACKET_LEN];
  strcpy((char*)message, np.CRTS_IP);
  srand(12);
  for (int i=0; i < packet_num_bytes; i++)
    packet_num_prs[i] = rand() & 0xff;
  
  // initialize sig_terminate flag and check return from socket call
  sig_terminate = 0;
  if (CRTS_client_sock < 0) {
    printf("CRTS failed to create client socket\n");
    sig_terminate = 1;
  }
  if (CRTS_server_sock < 0) {
    printf("CRTS failed to create server socket\n");
    sig_terminate = 1;
  }

  float t_step = 8.0*(float)CRTS_CR_NET_PACKET_LEN/np.net_mean_throughput;
  float tx_time_delta = 0;
  struct timeval tx_time;
  fd_set read_fds;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 1;

  // Poisson RV generator
  std::default_random_engine rand_generator;
  std::poisson_distribution<int> poisson_generator(1e6);
  int poisson_rv;
  
  // Wait for the start-time before beginning the scenario
  struct timeval tv;
  time_t time_s;
  stop_time_s = sp.start_time_s + sp.runTime;
  while (1) {
    Receive_command_from_controller(&TCP_controller, &sp, &np);
    gettimeofday(&tv, NULL);
    time_s = tv.tv_sec;
    if (time_s >= sp.start_time_s)
      break;
    if (sig_terminate)
      break;
    usleep(1e4);
  }

  if (np.cr_type == ecr) {
    // Start ECR
    dprintf("Starting ECR object...\n");
    ECR->start_rx();
    ECR->start_tx();
    ECR->start_ce();
  }
  
  // main loop
  while (time_s < stop_time_s && !sig_terminate) {
    // Listen for any updates from the controller (non-blocking)
    dprintf("CRTS: Listening to controller for command\n");
    Receive_command_from_controller(&TCP_controller, &sp, &np);

    // Send packets according to traffic model
	switch(np.net_traffic_type){
	case(NET_TRAFFIC_STREAM):
	  tx_time_delta += t_step*1e6;
	  break;
    case(NET_TRAFFIC_BURST):
	  tx_time_delta += t_step*np.net_burst_length*1e6;
	  break;
	case(NET_TRAFFIC_POISSON):
	  poisson_rv = poisson_generator(rand_generator);
	  tx_time_delta += t_step * (float)poisson_rv;
	  break;
	}
	tx_time.tv_sec = sp.start_time_s + (long int)floorf(tx_time_delta / 1e6);
    tx_time.tv_usec = (long int)fmod(tx_time_delta, 1e6);
    while (1) {
      gettimeofday(&tv, NULL);
      if ((tv.tv_sec == tx_time.tv_sec && tv.tv_usec > tx_time.tv_usec) ||
          tv.tv_sec > tx_time.tv_sec)
        break;      
    }

    // send burst of packets
    for(int i=0; i<np.net_burst_length; i++){
	  
	  // update packet number
	    packet_counter++;
	    for (int i=0; i < packet_num_bytes; i++)
          message[i+15] = ((packet_counter>>(8*(packet_num_bytes-i-1))) & 0xff )^packet_num_prs[i];
        
		// fill the rest with random data
        for (int i=15+packet_num_bytes; i < CRTS_CR_NET_PACKET_LEN; i++)
          message[i] = (rand() & 0xff);

	    // send UDP packet via CR
        dprintf("CRTS sending packet %i\n", packet_counter);
        int send_return =
            sendto(CRTS_client_sock, (char*)message, sizeof(message), 0,
                 (struct sockaddr *)&CRTS_client_addr, sizeof(CRTS_client_addr));
        if (send_return < 0)
          printf("Failed to send message\n");
	  
	    if (np.log_net_tx){
          log_tx_data(&sp, &np, send_return, packet_counter);
		}
	} 


    // read all available data from the UDP socket
    int recv_len = 0;
    FD_ZERO(&read_fds);
	FD_SET(CRTS_server_sock, &read_fds);
	while (select(CRTS_server_sock+1, &read_fds, NULL, NULL, &timeout) > 0 ){
	  recv_len = recvfrom(CRTS_server_sock, recv_buffer, recv_buffer_len, 0,
                          (struct sockaddr *)&CRTS_server_addr, &clientlen);
      
	  // determine packet number
	  int rx_packet_num = 0;
	  for (int i=0; i < packet_num_bytes; i++)
	    rx_packet_num += (((unsigned char)recv_buffer[15+i])^packet_num_prs[i]) << 8*(packet_num_bytes-i-1);

	  // print out/log details of received messages
      if (recv_len > 0) {
        // TODO: Say what address message was received from.
        // (It's in CRTS_server_addr)
        dprintf("CRTS received packet %i containing %i bytes:\n", rx_packet_num, recv_len);
        if (np.log_net_rx) {
          log_rx_data(&sp, &np, recv_len, rx_packet_num);
        }
      }
	
	  FD_ZERO(&read_fds);
	  FD_SET(CRTS_server_sock, &read_fds);
	}

    // Update the current time
    gettimeofday(&tv, NULL);
    time_s = tv.tv_sec;
  }

  // close the log files
  if (np.log_net_rx)
    log_rx_fstream.close();
  if (np.log_net_tx)
    log_tx_fstream.close();
 
  // close all network connections
  close(CRTS_client_sock);
  close(CRTS_server_sock);

  // auto-generate octave logs from binary logs
  char command[1000];
  if(np.generate_octave_logs){
    if(np.log_net_rx){
      sprintf(command, "./logs/logs2octave -c -l %s -N %d -n %d",
              net_rx_log_file_cpy, sp.totalNumReps, sp.repNumber);
      system(command);
    }

    if (np.log_net_tx) {
      sprintf(command, "./logs/logs2octave -C -l %s -N %d -n %d", 
	        net_tx_log_file_cpy, sp.totalNumReps, sp.repNumber );
      system(command);
    }
    
    if(np.log_phy_rx){
      sprintf(command, "./logs/logs2octave -r -l %s -N %d -n %d",
              np.phy_rx_log_file, sp.totalNumReps, sp.repNumber);
      system(command);
    }

    if(np.log_phy_tx){
      sprintf(command, "./logs/logs2octave -t -l %s -N %d -n %d",
              np.phy_tx_log_file, sp.totalNumReps, sp.repNumber);
      system(command);
    }
  }
  // auto-generate python logs from binary logs
  if(np.generate_python_logs){
    if(np.log_net_rx){
      sprintf(command, "./logs/logs2python -c -l %s -N %d -n %d",
              net_rx_log_file_cpy, sp.totalNumReps, sp.repNumber);
      system(command);
    }

    if (np.log_net_tx) {
      sprintf(command, "./logs/logs2octave -C -l %s -N %d -n %d", 
	       net_tx_log_file_cpy, sp.totalNumReps, sp.repNumber );
      system(command);
    }
    
    if(np.log_phy_rx){
      sprintf(command, "./logs/logs2python -r -l %s -N %d -n %d",
              np.phy_rx_log_file, sp.totalNumReps, sp.repNumber);
      system(command);
    }

    if(np.log_phy_tx){
      sprintf(command, "./logs/logs2python -t -l %s -N %d -n %d",
              np.phy_tx_log_file, sp.totalNumReps, sp.repNumber);
      system(command);
    }
  }


  // clean up ECR/python process
  if (np.cr_type == ecr) {
    delete ECR;
  } else if (np.cr_type == python) {
    kill(pid, SIGTERM);
  }

  printf(
      "CRTS: Reached termination. Sending termination message to controller\n");
  char term_message = CRTS_MSG_TERMINATE;
  write(TCP_controller, &term_message, 1);
  close(TCP_controller);
}
