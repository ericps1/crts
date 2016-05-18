#include <stdio.h>
#include <liquid/liquid.h>
#include <limits.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SC_Scoreboard.hpp"

struct feedback_struct
{
    int type;
    int node;
    float value;
};

// constructor
SC_Scoreboard::SC_Scoreboard() 
{
    // Create TCP client to CORNET3D
    TCP_Scoreboard = socket(AF_INET, SOCK_STREAM, 0);
    if (TCP_Scoreboard < 0) {
        printf("ERROR: Receiver Failed to Create Client Socket\n");
        exit(EXIT_FAILURE);
    }
    // Parameters for connecting to server
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(Scoreboard_IP);
    addr.sin_port = htons(Scoreboard_PORT);

    // Attempt to connect client socket to server
    int connect_status = 1;
    std::cout << "Waiting for connection from CORNET3D" << std::endl;
    while(connect_status != 0)
    {
        connect_status = connect(TCP_Scoreboard, (struct sockaddr *)&addr,
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
SC_Scoreboard::~SC_Scoreboard() {}

// setup feedback enables for each node
void SC_Scoreboard::initialize_node_fb() {

    // enable all feedback types
    int fb_enables = INT_MAX;
    //int fb_enables = INT_MAX;
    for(int i=0; i<sp.num_nodes; i++)
        set_node_parameter(i, CRTS_FB_EN, (void*) &fb_enables);

    double rx_stats_period = 1.0;
    for(int i=0; i<sp.num_nodes; i++){
        set_node_parameter(i, CRTS_RX_STATS, (void*) &rx_stats_period);
        set_node_parameter(i, CRTS_RX_STATS_FB, (void*) &rx_stats_period);
    }
}

// execute function
void SC_Scoreboard::execute() {
    //Only send feedback to CORNET3D when feedback is received from a node,
    //not when execute is called due to a timeout
    if(sc_event == FEEDBACK)
    {
        feedback_struct fs;
        switch (fb.fb_type) {
            case CRTS_TX_FREQ:
                fs.type = 1;
                fs.node = fb.node;
                fs.value = *(double*)fb.arg;
                fs.node = fb.node;
                send(TCP_Scoreboard, (char*)&fs, sizeof(fs), 0);
                printf("Node %i has updated it's transmit frequency to %.1e\n", fb.node, *(double*)fb.arg);
                break;
            case CRTS_TX_RATE:
                fs.type = 2;
                fs.node = fb.node;
                fs.value = *(double*)fb.arg;
                send(TCP_Scoreboard, (char*)&fs, sizeof(fs), 0);
                printf("Node %i has updated it's transmit rate to %.3e\n", fb.node, *(double*)fb.arg);
                break;
        }
    }
}




