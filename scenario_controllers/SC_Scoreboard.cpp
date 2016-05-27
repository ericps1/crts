#include <stdio.h>
#include <liquid/liquid.h>
#include <limits.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SC_Scoreboard.hpp"

#define TEST 1

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
SC_Scoreboard::~SC_Scoreboard() {
    delete[] old_frequencies;
    delete[] old_bandwidths;
    timer_destroy(switch_timer);
    for(int i = 0; i < sp.num_nodes; i++)
        timer_destroy(throughput_timers[i]);
    delete[] throughput_timers;
}

// setup feedback enables for each node
void SC_Scoreboard::initialize_node_fb() {
    old_frequencies = new double[sp.num_nodes];
    old_bandwidths = new double[sp.num_nodes];
    num_nodes_struct nns;
    nns.num_nodes = sp.num_nodes;
    printf("sizeof nns: %lu\n", sizeof(nns));
    send(TCP_Scoreboard, (char*)&nns, sizeof(nns), 0);
    for(int i = 0; i < sp.num_nodes; i++)
    {
        node_struct ns;
        ns.node = i;
        ns.frequency = np[i].tx_freq;
        old_frequencies[i] = ns.frequency;
        ns.bandwidth = np[i].tx_rate;
        old_bandwidths[i] = ns.bandwidth;
        strcpy(ns.team_name, np[i].team_name);
        if(np[i].type == CR)
            ns.role = 0;
        else
            ns.role = 1;
        printf("sending node %u\n", i);
        printf("size: %lu\n", sizeof(ns));
        send(TCP_Scoreboard, (char*)&ns, sizeof(ns), 0);
        printf("team: %s\n", np[i].team_name);
    }
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
    switch_timer = timer_create();
    timer_tic(switch_timer);
    throughput_timers = new timer[sp.num_nodes];
    for(int i = 0; i < sp.num_nodes; i++)
    {
        throughput_timers[i] = timer_create();
        timer_tic(throughput_timers[i]);
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
                fs.frequency = *(double*)fb.arg;
                fs.bandwidth = old_bandwidths[fb.node];
                old_frequencies[fb.node] = fs.frequency;
                send(TCP_Scoreboard, (char*)&fs, sizeof(fs), 0);
                printf("Node %i has updated it's transmit frequency to %.1e\n", fb.node, *(double*)fb.arg);
                break;
            case CRTS_TX_RATE:
                fs.type = 1;
                fs.node = fb.node;
                fs.bandwidth = *(double*)fb.arg;
                fs.frequency = old_frequencies[fb.node];
                old_bandwidths[fb.node] = fs.bandwidth;
                send(TCP_Scoreboard, (char*)&fs, sizeof(fs), 0);
                printf("Node %i has updated it's transmit rate to %.3e\n", fb.node, *(double*)fb.arg);
                break;
        }
    }
    if(timer_toc(throughput_timers[fb.node]) > 1.0)
    {
        timer_tic(throughput_timers[fb.node]);
        switch (fb.fb_type) 
        {
            case CRTS_RX_STATS:
                feedback_struct fs;
                struct ExtensibleCognitiveRadio::rx_statistics rx_stats = 
                    *(struct ExtensibleCognitiveRadio::rx_statistics*) fb.arg;
                float throughput = rx_stats.avg_throughput;
                // forward feedback to CORNET 3D web server
                fs.type = 2;
                fs.node = fb.node;
                //highjacking the frequency member of the struct to send the throughput
                fs.frequency = throughput;
                //put dummy data in the bandwidth field to make sure it lines up correctly in the stuct
                fs.bandwidth = 0.0;
                send(TCP_Scoreboard, (char*)&fs, sizeof(fs), 0);
                break;
        }
    }

    if(TEST == 1)
    {
        if(timer_toc(switch_timer) > 5)
        {
            if(flip == 1) 
            {
                test_frequency0 = 771e6;
                test_frequency1 = 774e6;
                test_bandwidth = 2e6;
            }
            else
            {
                test_frequency0 = 766e6;
                test_frequency1 = 768e6;
                test_bandwidth = 1e6;
            }
            set_node_parameter(0, CRTS_TX_FREQ, &test_frequency0);
            set_node_parameter(1, CRTS_RX_FREQ, &test_frequency0);

            set_node_parameter(0, CRTS_RX_FREQ, &test_frequency1);
            set_node_parameter(1, CRTS_TX_FREQ, &test_frequency1);

            set_node_parameter(0, CRTS_TX_RATE, &test_bandwidth);
            set_node_parameter(1, CRTS_RX_RATE, &test_bandwidth);
            
            set_node_parameter(0, CRTS_RX_RATE, &test_bandwidth);
            set_node_parameter(1, CRTS_TX_RATE, &test_bandwidth);
            
            flip *= -1;
            timer_tic(switch_timer);
        }
    }
}




