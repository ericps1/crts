#include <stdlib.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
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
//time_t start_time_s;
//struct node_parameters np;
    
void Receive_command_from_controller(int *TCP_controller, struct scenario_parameters *sp, struct node_parameters *np){
    // Listen to socket for message from controller
    char command_buffer[1+sizeof(time_t)+sizeof(struct node_parameters)];
    memset(&command_buffer, 0, sizeof(command_buffer));
    int rflag = recv(*TCP_controller, command_buffer, 1+sizeof(time_t)+sizeof(struct node_parameters), 0);
    
    dprintf("TCP receive flag: %i\n", rflag);
    int err = errno;
    if(rflag <= 0){
        if ((err == EAGAIN) || (err == EWOULDBLOCK))
            return;
        else{
            close(*TCP_controller);
            printf("Socket failure\n");
            exit(1);
        }
    }

    // Parse command
    switch (command_buffer[0]){
    case 's': // settings for upcoming scenario
        printf("CRTS: Received settings for scenario\n");
        // copy start time
        //memcpy((void*)&start_time_s, &command_buffer[1], sizeof(time_t));
        memcpy(sp, &command_buffer[1], sizeof(scenario_parameters));
        
        // copy node_parameters
        memcpy(np ,&command_buffer[1+sizeof(scenario_parameters)], sizeof(node_parameters));
        print_node_parameters(np);
        break;
    case 't': // terminate program
        printf("Received termination command from controller\n");
        exit(1);
    }
}

void Initialize_CR(struct node_parameters *np, void * ECR_p){
    
    // initialize ECR parameters if applicable
    if(np->cr_type == ecr)
    {
        ExtensibleCognitiveRadio *ECR = (ExtensibleCognitiveRadio *) ECR_p;
        
        // append relative locations for log files
        char rx_log_file_name[100];
        strcpy(rx_log_file_name, "./logs/bin/");
        strcat(rx_log_file_name, np->rx_log_file);
        strcat(rx_log_file_name, ".log");
        
		char tx_log_file_name[100];
        strcpy(tx_log_file_name, "./logs/bin/");
        strcat(tx_log_file_name, np->tx_log_file);
        strcat(tx_log_file_name, ".log");
        
        // set cognitive radio parameters
        ECR->set_ip(np->CRTS_IP);
        ECR->print_metrics_flag = np->print_metrics;
        ECR->log_rx_metrics_flag = np->log_rx_metrics;
        ECR->log_tx_parameters_flag = np->log_tx_parameters;
        ECR->set_ce_timeout_ms(np->ce_timeout_ms);
        strcpy(ECR->rx_log_file, rx_log_file_name);
        strcpy(ECR->tx_log_file, tx_log_file_name);
        //ECR->set_rx_freq(np->rx_freq-1e6, -1e6);
        ECR->set_rx_freq(np->rx_freq);
        ECR->set_rx_rate(np->rx_rate);
        ECR->set_rx_gain_uhd(np->rx_gain);
        ECR->set_rx_subcarriers(np->rx_subcarriers);
		ECR->set_rx_cp_len(np->rx_cp_len);
		ECR->set_rx_taper_len(np->rx_taper_len);
		//ECR->set_tx_freq(np->tx_freq-1e6, 1e6);
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
	}
    // intialize python radio if applicable
    else if(np->cr_type == python)
    {
        // set IP for TUN interface
        //char command[50];
        //sprintf(command, "ifconfig tunCRTS %s", np->CRTS_IP);
        //system("ip link set dev tunCRTS up");
        //sprintf(command, "ip addr add %s/24 dev tunCRTS", np->CRTS_IP);
        //system(command);
        //printf("Running command: %s\n", command);
        //system("route add -net 10.0.0.0 netmask 255.255.255.0 dev tunCRTS");
        //system("ifconfig");
    }
}

void log_rx_data(struct scenario_parameters *sp, struct node_parameters *np, int bytes){
    // update current time
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // open file, append parameters, and close
    std::ofstream log_fstream;
    log_fstream.open(np->CRTS_rx_log_file, std::ofstream::out|std::ofstream::binary|std::ofstream::app);
    if(log_fstream.is_open()){
        log_fstream.write((char*)&tv, sizeof(tv));
        log_fstream.write((char*)&bytes, sizeof(bytes));
    }
    else
        printf("Error opening log file: %s\n", np->CRTS_rx_log_file);

    log_fstream.close();
}

void uhd_quiet(uhd::msg::type_t type, const std::string &msg){}

void help_CRTS_CR() {
    printf("CRTS_CR -- Start a cognitive radio node. Only needs to be run explicitly when using CRTS_controller with -m option.\n");
    printf("        -- This program must be run from the main CRTS directory.\n");
    printf(" -h : Help.\n");
    //printf(" -t : Run Time - Length of time this node will run. In seconds.\n");
    //printf("      Default: 20.0 s\n");
    printf(" -a : IP Address of node running CRTS_controller.\n");
}

void terminate(int signum){
    printf("\nSending termination message to controller\n");
    sig_terminate = 1;
}


int main(int argc, char ** argv){
    
    // register signal handlers
    signal(SIGINT, terminate);
    signal(SIGQUIT, terminate);
    signal(SIGTERM, terminate);
    
    // timing variables
    //time_t runTime = 20;

    
    // Default IP address of controller
    char * controller_ipaddr = (char*) "192.168.1.56";

    int d;
    while((d = getopt(argc, argv, "ha:")) != EOF){
        switch(d){
        case 'h': help_CRTS_CR();               return 0;
        //case 't': runTime = atof(optarg);      break;
        case 'a': controller_ipaddr = optarg;   break;
        }
    }

    // Create TCP client to controller
    unsigned int controller_port = 4444;
    int TCP_controller = socket(AF_INET, SOCK_STREAM, 0);
    if (TCP_controller < 0)
    {
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
    int connect_status = connect(TCP_controller, (struct sockaddr*)&controller_addr, sizeof(controller_addr));
    if (connect_status){
        printf("Failed to Connect to server.\n");
        exit(EXIT_FAILURE);
    }
    dprintf("Connected to server\n");
    
    // Quiet UHD output
    uhd::msg::register_handler(&uhd_quiet);
    
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
    fcntl(TCP_controller, F_SETFL, O_NONBLOCK); // Set socket to non-blocking for future communication

	// copy log file name for post processing later
    char CRTS_rx_log_file_cpy[100];
    strcpy(CRTS_rx_log_file_cpy, np.CRTS_rx_log_file); 
	
	// modify log file name in node parameters for logging function
	char CRTS_rx_log_file[100];
    strcpy(CRTS_rx_log_file, "./logs/bin/");
    strcat(CRTS_rx_log_file, np.CRTS_rx_log_file);
    strcat(CRTS_rx_log_file, ".log");
	strcpy(np.CRTS_rx_log_file, CRTS_rx_log_file);
	
	// open CRTS rx log file to delete any current contents
    if (np.log_CRTS_rx_data){
        std::ofstream log_fstream;
        log_fstream.open(CRTS_rx_log_file, std::ofstream::out | std::ofstream::trunc);
        if (log_fstream.is_open())
        {
            log_fstream.close();
        }
        else
        {
            std::cout<<"Error opening log file:"<<CRTS_rx_log_file<<std::endl;
        }
    }

    // this is used to create a child process for python radios which can be killed later
    pid_t pid = 0;
    
    // pointer to ECR which may or may not be used
    ExtensibleCognitiveRadio *ECR = NULL;
    
    // Create and start the ECR or python CR so that they are in a ready
    // state when the experiment begins
    if(np.cr_type == ecr)
    {
        dprintf("Creating ECR object...\n");
        ECR = new ExtensibleCognitiveRadio;
        
        // set the USRP's timer to 0
        uhd::time_spec_t t0(0, 0, 1e6);
        ECR->usrp_rx->set_time_now(t0, 0);

        Initialize_CR(&np, (void*) ECR);
        
    }
    else if(np.cr_type == python)
    {
        dprintf("CRTS: Forking child process\n");
        pid_t pid = fork();
        
        // define child's process
        if(pid == 0){
            char command[2000] = "sudo python cognitive_radios/";
            strcat(command, np.python_file);
            for(int i = 0; i < np.num_arguments; i++)
            {
                strcat(command, " ");
                strcat(command, np.arguments[i]);
            }
            strcat(command, " &");
            int ret_value = system(command);
            if(ret_value != 0)
                std::cout << "error starting python radio" << std::endl;
        
            sleep(5);
            dprintf("CRTS Child: Initializing python CR\n");
            Initialize_CR(&np, NULL);

            while(true){
                if(sig_terminate) break;
            };

            dprintf("CRTS Child: Closing sockets\n");
            close(TCP_controller);
            exit(1);
        }
    }    

    // Define address structure for CRTS socket server used to receive network traffic
    struct sockaddr_in CRTS_server_addr;
    memset(&CRTS_server_addr, 0, sizeof(CRTS_server_addr));
    CRTS_server_addr.sin_family = AF_INET;
    // Only receive packets addressed to the CRTS_IP
    CRTS_server_addr.sin_addr.s_addr = inet_addr(np.CRTS_IP);
    CRTS_server_addr.sin_port = htons(port);
    socklen_t clientlen = sizeof(CRTS_server_addr);
    int CRTS_server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Define address structure for CRTS socket client used to send network traffic
    struct sockaddr_in CRTS_client_addr;
    memset(&CRTS_client_addr, 0, sizeof(CRTS_client_addr));
    CRTS_client_addr.sin_family = AF_INET;
    CRTS_client_addr.sin_addr.s_addr = inet_addr(np.TARGET_IP);
    CRTS_client_addr.sin_port = htons(port);
    int CRTS_client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Bind CRTS server socket
    bind(CRTS_server_sock, (sockaddr*)&CRTS_server_addr, clientlen);

    // set CRTS sockets to non-blocking
    fcntl(CRTS_client_sock, F_SETFL, O_NONBLOCK);
    fcntl(CRTS_server_sock, F_SETFL, O_NONBLOCK);
    
    // Define a buffer for receiving and a temporary message for sending
    int recv_buffer_len = 8192*2;
    char recv_buffer[recv_buffer_len];
    char message[128]; 
    strcpy(message, "Test Message from "); 
    strcat(message, np.CRTS_IP);    
    for(int i = 0; i < 128; i++)
        message[i] = rand() & 0xff;
    
    // initialize sig_terminate flag and check return from socket call
    sig_terminate = 0;
    if(CRTS_client_sock<0){
        printf("CRTS failed to create client socket\n");
        sig_terminate = 1;
    }
    if(CRTS_server_sock<0){
        printf("CRTS failed to create server socket\n");
        sig_terminate = 1;
    }

    // Wait for the start-time before beginning the scenario
    struct timeval tv;
    time_t time_s;
    time_t stop_time_s = sp.start_time_s + sp.runTime;
    while(1){
        gettimeofday(&tv, NULL);
        time_s = tv.tv_sec;
        if(time_s >= sp.start_time_s) 
            break;
    	if(sig_terminate)
			break;
	}

	if(np.cr_type == ecr){
        // Start ECR
        dprintf("Starting ECR object...\n");
        ECR->start_rx();
		ECR->start_tx();
		ECR->start_ce();
    }
    
    // main loop
    float tx_time_delta = 0;
    struct timeval tx_time;
    while(time_s < stop_time_s && !sig_terminate){
        // Listen for any updates from the controller (non-blocking)
        dprintf("CRTS: Listening to controller for command\n");
        Receive_command_from_controller(&TCP_controller, &sp, &np);

        // Wait (used for test purposes only)
        //usleep(np.tx_delay_us);
        tx_time_delta += np.tx_delay_us;
        tx_time.tv_sec = sp.start_time_s + (long int) floorf(tx_time_delta/1e6);
        tx_time.tv_usec = fmod(tx_time_delta, 1e6);
         while(1){
            gettimeofday(&tv, NULL);
            if((tv.tv_sec == tx_time.tv_sec && tv.tv_usec> tx_time.tv_usec) || tv.tv_sec > tx_time.tv_sec ) 
                break;
        }
        
        // send UDP packet via CR
        dprintf("CRTS: Sending UDP packet using CRTS client socket\n");
        int send_return = sendto(CRTS_client_sock, message, sizeof(message), 0, (struct sockaddr*)&CRTS_client_addr, sizeof(CRTS_client_addr));    
        if(send_return < 0) printf("Failed to send message\n");
        
        // read all available data from the UDP socket
        int recv_len =0;
        dprintf("CRTS: Reading from CRTS server socket\n");
        recv_len = recvfrom(CRTS_server_sock, recv_buffer, recv_buffer_len, 0, (struct sockaddr *)&CRTS_server_addr, &clientlen);
        // print out received messages
        if(recv_len > 0){
            // TODO: Say what address message was received from.
            // (It's in CRTS_server_addr)
            dprintf("\nCRTS received message:\n");
            for(int j=0; j<recv_len; j++)
                dprintf("%c", recv_buffer[j]);
            if(np.log_CRTS_rx_data){
				log_rx_data(&sp, &np, recv_len);
			}
        }
        
        // Update the current time
        gettimeofday(&tv, NULL);
        time_s = tv.tv_sec;    
    }

    printf("CRTS: Reached termination. Sending termination message to controller\n");
    char term_message = 't';
    write(TCP_controller, &term_message, 1);

    // clean up ECR/python process
    if(np.cr_type == ecr){
        delete ECR;
    }
    else if(np.cr_type == python){
        kill(pid, SIGTERM);
    }
	
	if(np.generate_octave_logs){
        char command[50];
        
        if(np.log_CRTS_rx_data){
            strcpy(command, "./logs/logs2octave -c -l ");
            strcat(command, CRTS_rx_log_file_cpy);
            system(command);
        }

        if(np.log_rx_metrics){
            strcpy(command, "./logs/logs2octave -r -l ");
            strcat(command, np.rx_log_file);
            system(command);
        }

        if(np.log_tx_parameters){
            strcpy(command, "./logs/logs2octave -t -l ");
            strcat(command, np.tx_log_file);
            system(command);
        }
    }

    close(TCP_controller);
    close(CRTS_client_sock);
    close(CRTS_server_sock);
    
}

