#include <stdio.h>
#include <liquid/liquid.h>
#include <limits.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SC_CORNET_Display.hpp"


// constructor
SC_CORNET_Display::SC_CORNET_Display(int argc, char** argv) {
  // Create TCP client to CORNET3D
  TCP_CORNET_Display = socket(AF_INET, SOCK_STREAM, 0);
  if (TCP_CORNET_Display < 0) {
    printf("ERROR: Receiver Failed to Create Client Socket\n");
    exit(EXIT_FAILURE);
  }
  // Parameters for connecting to server
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(CORNET_Display_IP);
  addr.sin_port = htons(CORNET_Display_PORT);

  // Attempt to connect client socket to server
  int connect_status = 1;
  std::cout << "Waiting for connection from CORNET3D" << std::endl;
  while(connect_status != 0)
  {
      connect_status = connect(TCP_CORNET_Display, (struct sockaddr *)&addr,
              sizeof(addr));
      if(connect_status != 0)
      {
          sleep(1);
      }

  }
  if (connect_status) {
      printf("Failed to Connect to server.\n");
      exit(EXIT_FAILURE);
  }
}

// destructor
SC_CORNET_Display::~SC_CORNET_Display() 
{
    delete old_frequencies;
    delete old_bandwidths;
}
// setup feedback enables for each node
void SC_CORNET_Display::initialize_node_fb() {
    old_frequencies = new double[sp.num_nodes];
    old_bandwidths = new double[sp.num_nodes];

    num_nodes_struct nns;
    nns.num_nodes = sp.num_nodes;
    printf("sizeof nns: %lu\n", sizeof(nns));
    send(TCP_CORNET_Display, (char*)&nns, sizeof(nns), 0);
    for(int i = 0; i < sp.num_nodes; i++)
    {
        node_struct ns;
        ns.node = i;
        ns.frequency = np[i].tx_freq;
        old_frequencies[i] = ns.frequency;
        ns.bandwidth = np[i].tx_rate;
        old_bandwidths[i] = ns.bandwidth;
        strcpy(ns.team_name, np[i].team_name);
        if(np[i].node_type == COGNITIVE_RADIO)
            ns.role = 0;
        else
            ns.role = 1;
        printf("sending node %u\n", i);
        printf("size: %lu\n", sizeof(ns));
        send(TCP_CORNET_Display, (char*)&ns, sizeof(ns), 0);
        printf("team: %s\n", np[i].team_name);
    }
    // enable all feedback types
    int fb_enables = INT_MAX;
    //int fb_enables = INT_MAX;
    for(int i=1; i<=sp.num_nodes; i++)
        set_node_parameter(i, CRTS_FB_EN, (void*) &fb_enables);

    double rx_stats_period = 3.0;
    double rx_stats_report_rate = 1.0;
    for(int i=1; i<=sp.num_nodes; i++)
    {
        set_node_parameter(i, CRTS_RX_STATS, (void*) &rx_stats_period);
        set_node_parameter(i, CRTS_RX_STATS_FB, (void*) &rx_stats_report_rate);
    }
}

// execute function
void SC_CORNET_Display::execute() {
    //Only send feedback to CORNET3D when feedback is received from a node,
    //not when execute is called due to a timeout
    if(sc_event == FEEDBACK)
    {
        feedback_struct fs;
        switch (fb.fb_type) 
        {
          case CRTS_TX_FREQ:
              fs.type = 1;
              fs.node = fb.node;
              fs.frequency = *(double*)fb.arg;
              fs.bandwidth = old_bandwidths[fb.node];
              old_frequencies[fb.node] = fs.frequency;
              send(TCP_CORNET_Display, (char*)&fs, sizeof(fs), 0);
              printf("Node %i has updated it's transmit frequency to %.1e\n", fb.node, *(double*)fb.arg);
              break;
          case CRTS_TX_RATE:
              fs.type = 1;
              fs.node = fb.node;
              fs.bandwidth = *(double*)fb.arg;
              fs.frequency = old_frequencies[fb.node];
              old_bandwidths[fb.node] = fs.bandwidth;
              send(TCP_CORNET_Display, (char*)&fs, sizeof(fs), 0);
              printf("Node %i has updated it's transmit rate to %.3e\n", fb.node, *(double*)fb.arg);
              break;
          case CRTS_RX_STATS:
                struct ExtensibleCognitiveRadio::rx_statistics rx_stats = 
                    *(struct ExtensibleCognitiveRadio::rx_statistics*) fb.arg;
                float throughput = rx_stats.throughput;
                fs.type = 2;
                fs.node = fb.node;
                fs.frequency = throughput;
                fs.bandwidth = 0.0;
                send(TCP_CORNET_Display, (char*)&fs, sizeof(fs), 0);
                printf("node: %u, throughput: %f\n", fb.node, throughput);
                break;
        }
    }
}




