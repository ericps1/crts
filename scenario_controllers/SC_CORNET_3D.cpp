#include <stdio.h>
#include <liquid/liquid.h>
#include <limits.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SC_CORNET_3D.hpp"

struct crts_params
{
    int guard; //not used, needed to make struct correct size
    int type;
    int mod;
    int crc;
    int fec0;
    int fec1;
    double freq;
    double bandwidth;
    double gain;
};
// constructor
SC_CORNET_3D::SC_CORNET_3D() 
{
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
    int connect_status = 1;
    std::cout << "Waiting for connection from CORNET3D" << std::endl;
    while(connect_status != 0)
    {
        connect_status = connect(TCP_CORNET_3D, (struct sockaddr *)&addr,
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
SC_CORNET_3D::~SC_CORNET_3D() {}

// setup feedback enables for each node
void SC_CORNET_3D::initialize_node_fb() {

    // enable all feedback types
    int fb_enables = CRTS_RX_STATS_FB_EN;
    //int fb_enables = INT_MAX;
    for(int i=0; i<sp.num_nodes; i++)
        set_node_parameter(i, CRTS_FB_EN, (void*) &fb_enables);

    double rx_stats_period = .1;
    for(int i=0; i<sp.num_nodes; i++){
        set_node_parameter(i, CRTS_RX_STATS, (void*) &rx_stats_period);
        set_node_parameter(i, CRTS_RX_STATS_FB, (void*) &rx_stats_period);
    }
}

// execute function
void SC_CORNET_3D::execute() {
    //Only send feedback to CORNET3D when feedback is received from a node,
    //not when execute is called due to a timeout
    if(sc_event == FEEDBACK)
    {
        char msg[100];
        msg[0] = CRTS_MSG_FEEDBACK;
        msg[1] = fb.node;
        msg[2] = fb.fb_type;

        int arg_len = get_feedback_arg_len(fb.fb_type);

        struct ExtensibleCognitiveRadio::rx_statistics* stats = (struct ExtensibleCognitiveRadio::rx_statistics*)fb.arg;
        float per = stats->avg_per;
        memcpy((void*) &msg[3], fb.arg, arg_len);

        std::string s_per = std::to_string(per);
        if(fb.node == 0)
        {
            // forward feedback to CORNET 3D web server
            send(TCP_CORNET_3D, s_per.c_str(), sizeof(s_per), 0);
        }
    }
    // forward commands from CORNET 3D webserver to node
    fd_set fds;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100;
    FD_ZERO(&fds);
    FD_SET(TCP_CORNET_3D, &fds);
    crts_params params;
    if(select(TCP_CORNET_3D+1, &fds, NULL, NULL, &timeout)) 
    {
        int rlen = recv(TCP_CORNET_3D, &params, sizeof(params), 0);
        if(rlen > 0)
        {
            /*
            std::cout << "node: " << fb.node << std::endl;
            printf("params.mod: %u\n", params.mod);
            printf("params.crc: %u\n", params.crc);
            printf("params.fec0: %u\n", params.fec0);
            printf("params.fec1: %u\n", params.fec1);
            printf("params.freq: %f\n", params.freq);
            printf("params.bandwidth: %f\n", params.bandwidth);
            printf("params.gain: %f\n", params.gain);
            */

            if(params.mod != old_mod)
            {
                set_node_parameter(0, CRTS_TX_MOD, &params.mod);  
                old_mod = params.mod;
            }

            if(params.crc != old_crc)
            {
                set_node_parameter(0, CRTS_TX_CRC, &params.crc);
                old_crc = params.crc;
            }

            if(params.fec0 != old_fec0)
            {
                set_node_parameter(0, CRTS_TX_FEC0, &params.fec0);
                old_fec0 = params.fec0;
            }   

            if(params.fec1 != old_fec1)
            {
                set_node_parameter(0, CRTS_TX_FEC1, &params.fec1);
                old_fec1 = params.fec1;
            }

            if(params.freq != old_freq)
            {
                set_node_parameter(0, CRTS_TX_FREQ, &params.freq);
                set_node_parameter(1, CRTS_RX_FREQ, &params.freq);
                old_freq = params.freq;
            }

            if(params.bandwidth != old_bandwidth)
            {
                set_node_parameter(0, CRTS_TX_RATE, &params.bandwidth);
                set_node_parameter(1, CRTS_RX_RATE, &params.bandwidth);
                old_bandwidth = params.bandwidth;
            }

            if(params.gain != old_gain)
            {
                set_node_parameter(0, CRTS_TX_GAIN, &params.gain);
                old_gain = params.gain;
            }
        }
    }
}









