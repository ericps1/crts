#include <stdio.h>
#include <liquid/liquid.h>
#include <limits.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SC_CORNET_Tutorial.hpp"


// constructor
SC_CORNET_Tutorial::SC_CORNET_Tutorial(int argc, char** argv) {
  // Create TCP client to CORNET3D
  TCP_CORNET_Tutorial = socket(AF_INET, SOCK_STREAM, 0);
  if (TCP_CORNET_Tutorial < 0) {
    printf("ERROR: Receiver Failed to Create Client Socket\n");
    exit(EXIT_FAILURE);
  }
  // Parameters for connecting to server
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(CORNET_Tutorial_IP);
  addr.sin_port = htons(CORNET_Tutorial_PORT);

  // Attempt to connect client socket to server
  int connect_status = 1;
  std::cout << "Waiting for connection from CORNET3D" << std::endl;
  while(connect_status != 0)
  {
      connect_status = connect(TCP_CORNET_Tutorial, (struct sockaddr *)&addr,
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
  old_mod = 40;
  old_crc = 6;
  old_fec0 = 12;
  old_fec1 = 1;
  old_freq = 770e6;
  old_bandwidth = 1e6;
  old_gain = 20.0;
}

// destructor
SC_CORNET_Tutorial::~SC_CORNET_Tutorial() {}

// setup feedback enables for each node
void SC_CORNET_Tutorial::initialize_node_fb() {

    // enable all feedback types
    int fb_enables = INT_MAX;
    //int fb_enables = INT_MAX;
    for(int i=1; i<=sp.num_nodes; i++)
        set_node_parameter(i, CRTS_FB_EN, (void*) &fb_enables);

    double rx_stats_period = 1.0;
    double rx_stats_report_rate = 1.0;
    set_node_parameter(2, CRTS_RX_STATS, (void*) &rx_stats_period);
    set_node_parameter(2, CRTS_RX_STATS_FB, (void*) &rx_stats_report_rate);
}

// execute function
void SC_CORNET_Tutorial::execute() {
    //Only send feedback to CORNET3D when feedback is received from a node,
    //not when execute is called due to a timeout
    if(sc_event == FEEDBACK)
    {
        feedback_struct fs;
        switch (fb.fb_type) {
            case CRTS_TX_FREQ:
                printf("received frequency update\n");
                break;
            case CRTS_RX_STATS:
                struct ExtensibleCognitiveRadio::rx_statistics rx_stats = 
                    *(struct ExtensibleCognitiveRadio::rx_statistics*) fb.arg;
                float per = rx_stats.per;
                float throughput = rx_stats.throughput;
                // forward feedback to CORNET 3D web server
                fs.type = 0;
                fs.node = fb.node;
                fs.frequency = per;
                fs.bandwidth = throughput;
                send(TCP_CORNET_Tutorial, (char*)&fs, sizeof(fs), 0);
                break;
                
        }
    }
    // forward commands from CORNET 3D webserver to node
    fd_set fds;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100;
    FD_ZERO(&fds);
    FD_SET(TCP_CORNET_Tutorial, &fds);
    crts_signal_params params;
    if(select(TCP_CORNET_Tutorial+1, &fds, NULL, NULL, &timeout)) 
    {
        int rlen = recv(TCP_CORNET_Tutorial, &params, sizeof(params), 0);
        if(rlen > 0)
        {
            
            printf("params.node: %u\n", params.node);
            printf("params.mod: %u\n", params.mod);
            printf("params.crc: %u\n", params.crc);
            printf("params.fec0: %u\n", params.fec0);
            printf("params.fec1: %u\n", params.fec1);
            printf("params.freq: %f\n", params.freq);
            printf("params.bandwidth: %f\n", params.bandwidth);
            printf("params.gain: %f\n", params.gain);
            
            //CORNET3D backend sends a 9 when client disconnects
            //Call killall crts_controller with in turn shuts down all nodes
            if(params.type == 9)
            {
                printf("calling killall\n");
                system("killall crts_controller");
                exit(1);
            }

            if(params.mod >= 0 && params.mod != old_mod)
            {
                set_node_parameter(params.node, CRTS_TX_MOD, &params.mod);  
                old_mod = params.mod;
            }

            if(params.crc >= 0 && params.crc != old_crc)
            {
                set_node_parameter(params.node, CRTS_TX_CRC, &params.crc);
                old_crc = params.crc;
            }

            if(params.fec0 >= 0 && params.fec0 != old_fec0)
            {
                set_node_parameter(params.node, CRTS_TX_FEC0, &params.fec0);
                old_fec0 = params.fec0;
            }   

            if(params.fec1 >= 0 && params.fec1 != old_fec1)
            {
                set_node_parameter(params.node, CRTS_TX_FEC1, &params.fec1);
                old_fec1 = params.fec1;
            }

            if(params.freq >= 0 && params.freq != old_freq)
            {
                set_node_parameter(params.node, CRTS_TX_FREQ, &params.freq);
                set_node_parameter(params.node == 1 ? 2 : 1, CRTS_RX_FREQ, &params.freq);
                old_freq = params.freq;
            }

            if(params.bandwidth >= 0 && params.bandwidth != old_bandwidth)
            {
                set_node_parameter(params.node, CRTS_TX_RATE, &params.bandwidth);
                set_node_parameter(params.node == 1 ? 2 : 1, CRTS_RX_RATE, &params.bandwidth);
                old_bandwidth = params.bandwidth;
            }

            if(params.gain >= 0 && params.gain != old_gain)
            {
                set_node_parameter(params.node, CRTS_TX_GAIN, &params.gain);
                old_gain = params.gain;
            }
        }
    }
}




