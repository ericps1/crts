#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <string>
#include <time.h>
#include <uhd/utils/msg.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <errno.h>
#include <signal.h>
#include <complex>
#include <liquid/liquid.h>
#include <fstream>
#include "CRTS.hpp"
#include "interferer.hpp"
#include "node_parameters.hpp"
#include "read_configs.hpp"
#include "timer.h"

#define DEBUG 0
#if DEBUG == 1
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

// global variables
time_t stop_time_s;
struct timeval tv;
time_t time_s;
int sig_terminate;
int time_terminate;
int TCP_controller;

// ========================================================================
//  FUNCTION:  Receive_command_from_controller
// ========================================================================
static inline void
Receive_command_from_controller(Interferer *Int, struct node_parameters *np,
                                struct scenario_parameters *sp) {
  fd_set fds;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(TCP_controller, &fds);

  // Listen to socket for message from controller
  if (select(TCP_controller + 1, &fds, NULL, NULL, &timeout)) {
    char command_buffer[1 + sizeof(struct scenario_parameters) +
                        sizeof(struct node_parameters)];
    int rflag = recv(TCP_controller, command_buffer,
                     1 + sizeof(struct scenario_parameters) +
                         sizeof(struct node_parameters),
                     0);
    int err = errno;
    if (rflag <= 0) {
      if ((err == EAGAIN) || (err == EWOULDBLOCK)) {
        return;
      } else {
        close(TCP_controller);
        printf("Socket failure\n");
        exit(1);
      }
    }

    // Parse command
    switch (command_buffer[0]) {
    case CRTS_MSG_SCENARIO_PARAMETERS: // settings for upcoming scenario
    {
      dprintf("Received settings for scenario\n");

      // copy scenario parameters
      memcpy(sp, &command_buffer[1], sizeof(struct scenario_parameters));

      // copy node_parameters
      memcpy(np, &command_buffer[1 + sizeof(struct scenario_parameters)],
             sizeof(struct node_parameters));
      print_node_parameters(np);

      // set interferer parameters
      Int->tx_freq = np->tx_freq;
      Int->interference_type = np->interference_type;
      Int->period = np->period;
      Int->duty_cycle = np->duty_cycle;
      Int->tx_rate = np->tx_rate;
      Int->tx_gain = np->tx_gain;
	  Int->tx_gain_soft = np->tx_gain_soft;

      // set USRP settings
      Int->usrp_tx->set_tx_freq(Int->tx_freq);
      Int->usrp_tx->set_tx_rate(Int->tx_rate);
      Int->usrp_tx->set_tx_gain(Int->tx_gain);

      // set freq parameters
      Int->tx_freq_behavior = np->tx_freq_behavior;
	  Int->tx_freq_min = np->tx_freq_min;
      Int->tx_freq_max = np->tx_freq_max;
      Int->tx_freq_dwell_time = np->tx_freq_dwell_time;
      Int->tx_freq_resolution = np->tx_freq_resolution;
      Int->tx_freq_bandwidth = floor(np->tx_freq_max - np->tx_freq_min); 

      // FIXME
      // If log file names use subdirectories, create them if they don't exist
      //char * subdirptr_tx = strrchr(phy_tx_log_file, '/');
      //if (subdirptr_tx) {
      //  char subdirs_tx[60];
      //  // Get the names of the subdirectories
      //  strncpy(subdirs_tx, phy_tx_log_file, subdirptr_tx - phy_tx_log_file);
      //  subdirs_tx[subdirptr_tx - phy_tx_log_file] = '\0';
      //  char mkdir_cmd[100];
      //  strcpy(mkdir_cmd, "mkdir -p ./logs/bin/");
      //  strcat(mkdir_cmd, subdirs_tx);
      //  // Create them
      //  system(mkdir_cmd);
      //}
      
      // create string of actual log file location
      char tx_log_full_path[100];
      strcpy(tx_log_full_path, "./logs/bin/");
      strcat(tx_log_full_path, np->phy_tx_log_file);
      strcat(tx_log_full_path, ".log");
      Int->set_log_file(tx_log_full_path);
      Int->log_tx_flag = np->log_phy_tx;
      
      break;
    }

    case CRTS_MSG_MANUAL_START: // updated start time (used for manual mode)
      dprintf("Received an updated start time\n");
      memcpy(&sp->start_time_s, &command_buffer[1], sizeof(time_t));
      stop_time_s = sp->start_time_s + sp->runTime;
      break;
    case CRTS_MSG_TERMINATE: // terminate program
      dprintf("Received termination command from controller\n");
      sig_terminate = 1;
	  break;
    }
  }
}

// ========================================================================
//  FUNCTION:  uhd_quiet
// ========================================================================
void uhd_quiet(uhd::msg::type_t type, const std::string &msg) {}

// ========================================================================
//  FUNCTION:  help_CRTS_interferer
// ========================================================================
void help_CRTS_interferer() {
  printf("CRTS_interferer -- Start a cognitive radio interferer node. Only "
         "needs to be run explicitly when using CRTS_controller with -m "
         "option.\n");
  printf("                -- This program must be run from the main CRTS "
         "directory.\n");
  printf(" -h : Help.\n");
  printf(" -t : Run Time - Length of time this node will run. In seconds.\n");
  printf("      Default: 20.0 s\n");
  printf(" -a : IP Address of node running CRTS_controller.\n");
  printf("      Default: 192.168.1.56.\n");
}

// ========================================================================
//  FUNCTION:  terminate
// ========================================================================
void terminate(int signum) { sig_terminate = 1; }

// ==========================================================================
// ==========================================================================
// ==========================================================================
//  MAIN PROGRAM
// ==========================================================================
// ==========================================================================
// ==========================================================================
int main(int argc, char **argv) {
  
  // register signal handlers
  signal(SIGINT, terminate);
  signal(SIGQUIT, terminate);
  signal(SIGTERM, terminate);

  // set default values
  time_t run_time = DEFAULT_RUN_TIME;
  char *controller_ipaddr = (char *)DEFAULT_CONTROLLER_IP_ADDRESS;
  TCP_controller = socket(AF_INET, SOCK_STREAM, 0);

  // validate TCP Controller
  if (TCP_controller < 0) {
    printf("ERROR: Receiver Failed to Create Client Socket\n");
    exit(EXIT_FAILURE);
  }

  // get command line parameters
  int d;
  while ((d = getopt(argc, argv, "ht:a:")) != EOF) {
    switch (d) {
    case 'h':
      help_CRTS_interferer();
      return 0;
    case 't':
      run_time = atof(optarg);
      break;
    case 'a':
      controller_ipaddr = optarg;
      break;
    }
  }

  // Parameters for connecting to server
  struct sockaddr_in controller_addr;
  memset(&controller_addr, 0, sizeof(controller_addr));
  controller_addr.sin_family = AF_INET;
  controller_addr.sin_addr.s_addr = inet_addr(controller_ipaddr);
  controller_addr.sin_port = htons(CRTS_TCP_CONTROL_PORT);

  // Attempt to connect client socket to server
  int connect_status =
      connect(TCP_controller, (struct sockaddr *)&controller_addr,
              sizeof(controller_addr));
  if (connect_status) {
    printf("Failed to Connect to server.\n");
    exit(EXIT_FAILURE);
  }

  uhd::msg::register_handler(&uhd_quiet);

  // Create node parameters struct and interferer object
  struct node_parameters np;
  struct scenario_parameters sp;
  Interferer *Int;
  Int = new Interferer;
  
  // Read initial scenario info from controller
  dprintf("Receiving command from controller\n");
  Receive_command_from_controller(Int, &np, &sp);

  sig_terminate = 0;
  
  // wait for start time and calculate stop time
  stop_time_s = sp.start_time_s + run_time;
  while (1) {
    Receive_command_from_controller(Int, &np, &sp);
    gettimeofday(&tv, NULL);
    time_s = tv.tv_sec;
    if (time_s >= sp.start_time_s)
      break;
  }

  sleep(2);
  dprintf("Starting interferer\n");
  Int->start_tx();
  
  // loop until end of scenario
  while (!sig_terminate && time_s < stop_time_s) {
    Receive_command_from_controller(Int, &np, &sp);

    if (sig_terminate)
      break;

    // update current time
    gettimeofday(&tv, NULL);
    time_s = tv.tv_sec;
  }
  
  Int->stop_tx();
  sleep(1);
  delete Int;

  if(np.generate_octave_logs) {
    char command[100];
    sprintf(command, "./logs/logs2octave -i -l %s -N %d -n %d",
            np.phy_tx_log_file, sp.totalNumReps, sp.repNumber);
    printf("command: %s\n", command);
	system(command);
  }
  
  dprintf("Sending termination message to controller\n");
  char term_message = CRTS_MSG_TERMINATE;
  write(TCP_controller, &term_message, 1);
}
