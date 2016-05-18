#include <stdio.h>
#include <liquid/liquid.h>
#include <limits.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SC_CORNET_3D.hpp"

// constructor
SC_CORNET_3D::SC_CORNET_3D(int argc, char **argv) {
  // Create TCP client to CORNET3D
  TCP_CORNET_3D = socket(AF_INET, SOCK_STREAM, 0);
  if (TCP_CORNET_3D < 0) {
    printf("ERROR: Receiver Failed to Create Client Socket\n");
    exit(EXIT_FAILURE);
  }
  // Parameters for connecting to server
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(CORNET_3D_IP);
  addr.sin_port = htons(CORNET_3D_PORT);

  // Attempt to connect client socket to server
  int connect_status =
      connect(TCP_CORNET_3D, (struct sockaddr *)&addr,
              sizeof(addr));
  if (connect_status) {
    printf("Failed to Connect to server.\n");
    exit(EXIT_FAILURE);
  }
}

// destructor
SC_CORNET_3D::~SC_CORNET_3D() {}

// setup feedback enables for each node
void SC_CORNET_3D::initialize_node_fb() {
  
  // enable all feedback types
  int fb_enables = INT_MAX;
  for(int i=0; i<sp.num_nodes; i++)
    set_node_parameter(i, CRTS_FB_EN, (void*) &fb_enables);
  
  double rx_stats_period = 0.1;
  for(int i=0; i<sp.num_nodes; i++){
    set_node_parameter(i, CRTS_RX_STATS, (void*) &rx_stats_period);
    set_node_parameter(i, CRTS_RX_STATS_FB, (void*) &rx_stats_period);
  }
}

// execute function
void SC_CORNET_3D::execute(int node, char fb_type, void *_arg) {

  char msg[16];

  msg[0] = CRTS_MSG_FEEDBACK;
  msg[1] = node;
  msg[2] = fb_type;
  
  int arg_len = get_feedback_arg_len(fb_type);

  memcpy((void*) &msg[3], _arg, arg_len);

  // forward feedback to CORNET 3D web server
  send(TCP_CORNET_3D, msg, 3+arg_len, 0);

  // forward commands from CORNET 3D webserver to node
  fd_set fds;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 100;
  FD_ZERO(&fds);
  FD_SET(TCP_CORNET_3D, &fds);
  
  while(select(TCP_CORNET_3D+1, &fds, NULL, NULL, &timeout)) {
    int rlen = recv(TCP_CORNET_3D, msg, 1, 0);
    if (rlen < 0) {
      printf("CORNET 3D socket failure\n");
      exit(1);
    }
    // switch for different message types
    int node, cont_type, arg_len;
    switch (msg[0]) {
      case CRTS_MSG_CONTROL:
        rlen = recv(TCP_CORNET_3D, &msg[1], 2, 0);
        node = msg[1];
        cont_type = msg[2];
        arg_len = get_control_arg_len(cont_type);
        rlen = recv(TCP_CORNET_3D, &msg[3], arg_len, 0);
        set_node_parameter(node, cont_type, &msg[3]);
        break;
      default:
        printf("undefined message received from CORNET 3D web server\n");
    }
  }

  /*printf("\n");
  
  // handle all possible feedback types
  switch (fb_type) {
    case CRTS_TX_STATE:
      if (*(int*)_arg == TX_STOPPED)
        printf("Node %i has stopped transmitting\n", node);
      if (*(int*)_arg == TX_CONTINUOUS)
        printf("Node %i has started transmitting\n", node);
      break;
    case CRTS_TX_FREQ:
      printf("Node %i has updated it's transmit frequency to %.1e\n", node, *(double*)_arg);
      break;
    case CRTS_TX_RATE:
      printf("Node %i has updated it's transmit rate to %.3e\n", node, *(double*)_arg);
      break;
    case CRTS_TX_GAIN:
      printf("Node %i has updated it's transmit gain to %.3f\n", node, *(double*)_arg);
      break;
    case CRTS_TX_MOD:
      printf("Node %i has updated it's transmit modulation to %s\n", node, 
             modulation_types[*(int*)_arg].name);
      break;
    case CRTS_TX_FEC0:
      printf("Node %i has updated it's inner FEC scheme  to %s\n", node, 
             fec_scheme_str[*(int*)_arg][0]);
      break;
    case CRTS_TX_FEC1:
      printf("Node %i has updated it's outter FEC scheme to %s\n", node, 
             fec_scheme_str[*(int*)_arg][0]);
      break;
    
    case CRTS_RX_STATE:
      if (*(int*)_arg == RX_STOPPED)
        printf("Node %i has stopped receiving\n", node);
      if (*(int*)_arg == RX_CONTINUOUS)
        printf("Node %i has started receiving\n", node);
      break;
    case CRTS_RX_FREQ:
      printf("Node %i has updated it's receive frequency to %.3e\n", node, *(double*)_arg);
      break;
    case CRTS_RX_RATE:
      printf("Node %i has updated it's receive rate to %.3e\n", node, *(double*)_arg);
      break;
    case CRTS_RX_GAIN:
      printf("Node %i has updated it's receive gain to %.1f\n", node, *(double*)_arg);
      break;
    case CRTS_RX_STATS:
      struct ExtensibleCognitiveRadio::rx_statistics rx_stats = 
        *(struct ExtensibleCognitiveRadio::rx_statistics*) _arg;
      printf("Node %i has sent updated receive statistics:\n", node);
      printf("  Number of frames received: %i\n", rx_stats.frames_received);
      printf("  Average EVM:               %.3f\n", rx_stats.avg_evm);
      printf("  Average RSSI:              %.3f\n", rx_stats.avg_rssi);
      printf("  Average PER:               %.3f\n", rx_stats.avg_per);
      printf("  Average throughput:        %.3e\n", rx_stats.avg_throughput);
      break;
   }
   */
}



