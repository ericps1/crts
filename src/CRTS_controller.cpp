#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/time.h>
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
#include "CRTS.hpp"
#include "ECR.hpp"
#include "node_parameters.hpp"
#include "read_configs.hpp"

// EDIT INCLUDE START FLAG
#include "../scenario_controllers/SC_BER_Sweep.hpp"
#include "../scenario_controllers/SC_Control_and_Feedback_Test.hpp"
#include "../scenario_controllers/SC_CORNET_3D.hpp"
#include "../scenario_controllers/SC_Template.hpp"
// EDIT INCLUDE END FLAG

#define MAXPENDING 5

// global variables
int sig_terminate;
int num_nodes_terminated;

int receive_msg_from_nodes(int *TCP_nodes, int num_nodes, Scenario_Controller *SC) {
  // Listen to sockets for messages from any node
  char msg[256];
  for (int i = 0; i < num_nodes; i++) {
    int rflag = recv(TCP_nodes[i], msg, 1, 0);
    int err = errno;

    // Handle errors
    if (rflag <= 0) {
      if (!((err == EAGAIN) || (err == EWOULDBLOCK))) {
        close(TCP_nodes[i]);
        printf(
            "Node %i has disconnected. Terminating the current scenario..\n\n",
            i + 1);
        // tell all nodes to terminate program
        for (int j = 0; j < num_nodes; j++) {
          write(TCP_nodes[j], &msg, 1);
        }
        return 1;
      }
    }

    // Parse command if received a message
    else {
      switch (msg[0]) {
        case CRTS_MSG_TERMINATE: // terminate program
          printf("Node %i has sent a termination message...\n", i + 1);
          num_nodes_terminated++;
          // check if all nodes have terminated
          if (num_nodes_terminated == num_nodes)
            return 1;
          break;
        case CRTS_MSG_FEEDBACK:{
          // receive the number of feedback arguments sent
          rflag = recv(TCP_nodes[i], &msg[1], 1, 0);
          int fb_msg_ind = 2;
          // receive all feedback arguments
          for(int j=0; j<msg[1]; j++){
            rflag = recv(TCP_nodes[i], &msg[fb_msg_ind], 1, 0);
            int fb_arg_len = get_feedback_arg_len(msg[fb_msg_ind]);
            fb_msg_ind++;
            
            rflag = recv(TCP_nodes[i], &msg[fb_msg_ind], fb_arg_len, 0);
            SC->execute(i, msg[fb_msg_ind-1], (void *) &msg[fb_msg_ind]);
            fb_msg_ind += fb_arg_len;
          }
          break;
        }
        default:
          printf("Invalid message type received from node %i\n", i+1);
      }
    }
  }

  return 0;
}

void help_CRTS_controller() {
  printf("CRTS_controller -- Initiate cognitive radio testing.\n");
  printf(" -h : Help.\n");
  printf(" -m : Manual Mode - Start each node manually rather than have "
         "CRTS_controller do it automatically.\n");
  printf(" -f : Master scenario file (default: master_scenario_file.cfg).\n");
  printf(" -a : IP Address - IP address of this computer as seen by remote "
         "nodes.\n");
  printf("      Autodetected by default.\n");
}

void terminate(int signum) {
  printf("Terminating scenario on all nodes\n");
  sig_terminate = 1;
}

int main(int argc, char **argv) {

  // register signal handlers
  signal(SIGINT, terminate);
  signal(SIGQUIT, terminate);
  signal(SIGTERM, terminate);

  sig_terminate = 0;

  int manual_execution = 0;

  // Use current username as default username for ssh
  char ssh_uname[LOGIN_NAME_MAX + 1];
  getlogin_r(ssh_uname, LOGIN_NAME_MAX + 1);

  // Use currnet location of CRTS Directory as defualt for ssh
  char crts_dir[1000];
  getcwd(crts_dir, 1000);

  // Default name of master scenario file
  char *nameMasterScenFile = (char *)"master_scenario_file.cfg";

  // Default IP address of server as seen by other nodes
  char *serv_ip_addr;
  // Autodetect IP address as seen by other nodes
  struct ifreq interfaces;
  // Create a socket
  int fd_ip = socket(AF_INET, SOCK_DGRAM, 0);
  // For IPv4 Address
  interfaces.ifr_addr.sa_family = AF_INET;
  // Get Address associated with eth0
  strncpy(interfaces.ifr_name, "eth0", IFNAMSIZ - 1);
  ioctl(fd_ip, SIOCGIFADDR, &interfaces);
  close(fd_ip);
  // Get IP address out of struct
  char serv_ip_addr_auto[30];
  strcpy(serv_ip_addr_auto,
         inet_ntoa(((struct sockaddr_in *)&interfaces.ifr_addr)->sin_addr));
  serv_ip_addr = serv_ip_addr_auto;

  // interpret command line options
  int d;
  while ((d = getopt(argc, argv, "hf:ma:")) != EOF) {
    switch (d) {
    case 'h':
      help_CRTS_controller();
      return 0;
    case 'f':
      nameMasterScenFile = optarg;
      break;
    case 'm':
      manual_execution = 1;
      break;
    case 'a':
      serv_ip_addr = optarg;
      break;
    }
  }

  // Message about IP Address Detection
  printf("IP address of CRTS_controller autodetected as %s\n",
         serv_ip_addr_auto);
  printf("If this is incorrect, use -a option to fix.\n\n");

  // Create socket for incoming connections
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Transmitter Failed to Create Server Socket.\n");
    exit(EXIT_FAILURE);
  }
  // Allow reuse of a port. See
  // http://stackoverflow.com/questions/14388706/socket-options-so-reuseaddr-and-so-reuseport-how-do-they-differ-do-they-mean-t
  // FIXME: May not be necessary in this version of CRTS
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    printf("setsockopt() failed\n");
    exit(1);
  }
  // Construct local (server) address structure
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));          // Zero out structure
  serv_addr.sin_family = AF_INET;                    // Internet address family
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);     // Any incoming interface
  serv_addr.sin_port = htons(CRTS_TCP_CONTROL_PORT); // Local port
  // Bind to the local address to a port
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("ERROR: bind() error\n");
    exit(1);
  }

  // objects needs for TCP links to cognitive radio nodes
  int TCP_nodes[48];
  struct sockaddr_in nodeAddr[48];
  socklen_t node_addr_size[48];
  for (int i = 0; i < 48; i++)
    node_addr_size[i] = sizeof(nodeAddr[i]);
  struct node_parameters np[48];

  // read master scenario config file
  char scenario_name[251];
  char scenario_file[255];
  unsigned int scenario_reps;
  int num_scenarios = read_master_num_scenarios(nameMasterScenFile);
  printf("Number of scenarios: %i\n\n", num_scenarios);

  // variables for reading the system clock
  struct timeval tv;
  time_t time_s;
  time_t start_time_s;

  // loop through scenarios
  for (int i = 0; i < num_scenarios; i++) {

    // read scenario name and repetitions
    scenario_reps =
        read_master_scenario(nameMasterScenFile, i + 1, scenario_name);

    for (unsigned int scenRepNum = 1; scenRepNum <= scenario_reps;
         scenRepNum++) {
      printf("Scenario %i:\n", i + 1);
      printf("Rep %i:\n", scenRepNum);
      printf("Config file: %s\n", scenario_name);
      // read the scenario parameters from file
      strcpy(scenario_file, scenario_name);
      strcat(scenario_file, ".cfg");
      struct scenario_parameters sp = read_scenario_parameters(scenario_file);
      // Set the number of scenario  repititions in struct.
      sp.totalNumReps = scenario_reps;
      sp.repNumber = scenRepNum;

      printf("Number of nodes: %i\n", sp.num_nodes);
      printf("Run time: %lld\n", (long long)sp.runTime);
      printf("Scenario controller: %s\n", sp.SC);

      // create the scenario controller
      Scenario_Controller *SC;
      // EDIT SET SC START FLAG
      if(!strcmp(sp.SC, "SC_BER_Sweep"))
        SC = new SC_BER_Sweep();
      if(!strcmp(sp.SC, "SC_Control_and_Feedback_Test"))
        SC = new SC_Control_and_Feedback_Test();
      if(!strcmp(sp.SC, "SC_CORNET_3D"))
        SC = new SC_CORNET_3D();
      if(!strcmp(sp.SC, "SC_Template"))
        SC = new SC_Template();
      // EDIT SET SC END FLAG
    
      // determine the start time for the scenario based
      // on the current time and the number of nodes
      gettimeofday(&tv, NULL);
      time_s = tv.tv_sec;
      int pad_s = manual_execution ? 120 : 10;
      sp.start_time_s = time_s + 1 * sp.num_nodes + pad_s;

      // loop through nodes in scenario
      for (int j = 0; j < sp.num_nodes; j++) {
        char node_id[10];
        snprintf(node_id, 10, "%d", j + 1);

        // read in node settings
        memset(&np[j], 0, sizeof(struct node_parameters));
        printf("Reading node %i's parameters...\n", j + 1);
        np[j] = read_node_parameters(j + 1, scenario_file);
       
        // define log file names if they weren't defined by the scenario
        if (!strcmp(np[j].phy_rx_log_file, "")) {
          strcpy(np[j].phy_rx_log_file, scenario_name);
          sprintf(np[j].phy_rx_log_file, "%s_Node%i%s", np[j].phy_rx_log_file,
                  j + 1, "_CR_PHY_RX");
        }

        if (!strcmp(np[j].phy_tx_log_file, "")) {
          strcpy(np[j].phy_tx_log_file, scenario_name);
          sprintf(np[j].phy_tx_log_file, "%s_Node%i", np[j].phy_tx_log_file,
                  j + 1);
          switch (np[j].type) {
          case (CR):
            sprintf(np[j].phy_tx_log_file, "%s%s", np[j].phy_tx_log_file,
                    "_CR_PHY_TX");
            break;
          case (interferer):
            sprintf(np[j].phy_tx_log_file, "%s%s", np[j].phy_tx_log_file,
                    "_Int_PHY_TX");
            break;
          }
        }

        if (!strcmp(np[j].net_rx_log_file, "")) {
          strcpy(np[j].net_rx_log_file, scenario_name);
          sprintf(np[j].net_rx_log_file, "%s_Node%i%s", np[j].net_rx_log_file,
                  j + 1, "_CR_NET_RX");
        }

        if (!strcmp(np[j].net_tx_log_file, "")) {
          strcpy(np[j].net_tx_log_file, scenario_name);
          sprintf(np[j].net_tx_log_file, "%s_Node%i%s", np[j].net_tx_log_file,
                  j + 1, "_CR_NET_TX");
        }

        // append the rep number if necessary
        if (scenario_reps - 1) {
          sprintf(np[j].phy_rx_log_file, "%s_Rep%i", np[j].phy_rx_log_file,
                  scenRepNum);
          sprintf(np[j].phy_tx_log_file, "%s_Rep%i", np[j].phy_tx_log_file,
                  scenRepNum);
          sprintf(np[j].net_tx_log_file, "%s_Rep%i", np[j].net_tx_log_file,
                  scenRepNum);
          sprintf(np[j].net_rx_log_file, "%s_Rep%i", np[j].net_rx_log_file,
                  scenRepNum);
        }

        print_node_parameters(&np[j]);

        // send command to launch executable if not doing so manually
        int ssh_return = 0;
        if (!manual_execution) {
          char command[2000] = "ssh ";
          // Add username to ssh command
          strcat(command, ssh_uname);
          strcat(command, "@");
          strcat(command, np[j].CORNET_IP);
          strcat(command, " 'sleep 1 && ");
          strcat(command, " cd ");
          strcat(command, crts_dir);

          // add appropriate executable
          switch (np[j].type) {
          case CR:
            strcat(command, " && ./CRTS_CR");
            break;
          case interferer:
            strcat(command, " && ./CRTS_interferer");
            break;
          }

          // append IP Address of controller
          strcat(command, " -a ");
          strcat(command, serv_ip_addr);

          // set command to continue program after shell is closed
          strcat(command, " 2>&1 &'");
          strcat(command, " > ");
          strcat(command, crts_dir);
          strcat(command, "/node");
          strcat(command, node_id);
          strcat(command, ".SYSOUT &");
          ssh_return = system(command);
          // printf("Command executed: %s\n", command);
          // printf("Return value: %i\n", ssh_return);
        }

        if (ssh_return != 0) {
          printf("Failed to execute CRTS on node %i\n", j + 1);
          sig_terminate = 1;
          break;
        }

        // listen for node to connect to server
        if (listen(sockfd, MAXPENDING) < 0) {
          fprintf(stderr, "ERROR: Failed to Set Sleeping (listening) Mode\n");
          exit(EXIT_FAILURE);
        }

        // loop to accept
        // accept connection
        int accepted = 0;
        fd_set fds;
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        while (!sig_terminate && !accepted) {
          FD_ZERO(&fds);
          FD_SET(sockfd, &fds);
          if (select(sockfd + 1, &fds, NULL, NULL, &timeout) > 0) {
            TCP_nodes[j] = accept(sockfd, (struct sockaddr *)&nodeAddr[j],
                               &node_addr_size[j]);
            if (TCP_nodes[j] < 0) {
              fprintf(stderr, "ERROR: Sever Failed to Connect to Client\n");
              exit(EXIT_FAILURE);
            }

            accepted = 1;
          }
        }

        if (sig_terminate)
          break;

        // set socket to non-blocking
        fcntl(TCP_nodes[j], F_SETFL, O_NONBLOCK);

        // copy IP of connected node to node parameters if in manual mode
        if (manual_execution)
          strcpy(np[j].CORNET_IP, inet_ntoa(nodeAddr[j].sin_addr));

        // send scenario and node parameters
        printf("\nNode %i has connected. Sending its parameters...\n", j + 1);
        char msg_type = CRTS_MSG_SCENARIO_PARAMETERS;
        send(TCP_nodes[j], (void *)&msg_type, sizeof(char), 0);
        send(TCP_nodes[j], (void *)&sp, sizeof(struct scenario_parameters), 0);
        send(TCP_nodes[j], (void *)&np[j], sizeof(struct node_parameters), 0);
      
        // copy node parameters to the scenario controller object
        SC->np[j] = np[j];
      }

      // initialize feedback settings on nodes
      SC->TCP_nodes = TCP_nodes;
      SC->sp = sp;
      SC->initialize_node_fb();

      //sleep(5);
      
      // if in manual mode update the start time for all nodes
      if (manual_execution && !sig_terminate) {

        gettimeofday(&tv, NULL);
        start_time_s = tv.tv_sec + sp.num_nodes + 3;

        // send updated start time to all nodes
        char msg_type = CRTS_MSG_MANUAL_START;
        for (int j = 0; j < sp.num_nodes; j++) {
          send(TCP_nodes[j], (void *)&msg_type, sizeof(char), 0);
          send(TCP_nodes[j], (void *)&start_time_s, sizeof(time_t), 0);
        }
      } else {
        start_time_s = sp.start_time_s;
      }

      printf("Listening for scenario termination message from nodes\n");

      // main loop: wait for termination condition
      int time_terminate = 0;
      int msg_terminate = 0;
      num_nodes_terminated = 0;
      while ((!sig_terminate) && (!msg_terminate) && (!time_terminate)) {
        msg_terminate = receive_msg_from_nodes(&TCP_nodes[0], sp.num_nodes, SC);

        // check if the scenario should be terminated based on the elapsed time
        gettimeofday(&tv, NULL);
        time_s = tv.tv_sec;
        if (time_s > start_time_s + sp.runTime + 10)
          time_terminate = 1;
      }

      if (msg_terminate)
        printf("Ending scenario %i because all nodes have sent a "
               "termination message\n",
               i + 1);

      // terminate the current scenario on all nodes
      if (sig_terminate) {
        printf("Sending message to terminate nodes\n");
        // tell all nodes in the scenario to terminate the testing process
        char msg = CRTS_MSG_TERMINATE;
        for (int j = 0; j < sp.num_nodes; j++) {
          write(TCP_nodes[j], &msg, 1);
        }

        // get current time
        gettimeofday(&tv, NULL);
        time_t msg_sent_time_s = tv.tv_sec;

        // listen for confirmation that all nodes have terminated
        while ((!msg_terminate) && (!time_terminate)) {
          msg_terminate = receive_msg_from_nodes(&TCP_nodes[0], sp.num_nodes, SC);

          // check if the scenario should be terminated based on the elapsed
          // time
          gettimeofday(&tv, NULL);
          time_s = tv.tv_sec;
          if (time_s > msg_sent_time_s + 3) {
            printf("Nodes have not all responded with a successful "
                   "termination... forciblly terminating any CRTS processes "
                   "still running\n");
            time_terminate = 1;
          }
        }
      }

      // forcefully terminate all processes if one or more has failed to
      // terminate gracefully
      if (time_terminate) {
        for (int j = 0; j < sp.num_nodes; j++) {
          printf("Running CRTS_CR cleanup on node %i: %s\n", j + 1,
                 np[j].CORNET_IP);
          char command[2000] = "ssh ";

          // define command to execute CRTS_CR termination script
          strcat(command, ssh_uname);
          strcat(command, "@");
          strcat(command, np[j].CORNET_IP);
          strcat(command, " 'python ");
          strcat(command, crts_dir);
          strcat(command, "/src/CRTS_cr_terminate.py'");
          int ssh_return = system(command);

          if (ssh_return < 0)
            printf("Error command to terminate CRTS on node %i: %s", j,
                   np[j].CORNET_IP);
        }
      }
      
      // Close TCP Connections
      for (int j = 0; j < sp.num_nodes; j++) {
        close(TCP_nodes[j]);
      }

      delete SC;

      // don't continue to next scenario if there was a user issued termination
      if (sig_terminate)
        break;
      
    } // scenario repition loop

    // don't continue to next scenario if there was a user issued termination
    if (sig_terminate)
      break;

  } // scenario loop

} // main
