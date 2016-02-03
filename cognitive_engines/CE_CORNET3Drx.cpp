#include "ECR.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "CE_CORNET3Drx.hpp"
#include <sstream>
int newsockfdr = 0, sockfdr, portnor;
// constructor
CE_CORNET3Drx::CE_CORNET3Drx()
{
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    sockfdr = socket(AF_INET, SOCK_STREAM, 0);  // tcp socket
    if (sockfdr < 0)
        printf ("ERROR opening socket\n");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portnor = 6018;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnor);
    if (bind(sockfdr, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf ("error on binding\n");
        exit(1);
    }
    printf("waiting on connection\n");
    listen(sockfdr,2);
    clilen = sizeof(cli_addr);
    newsockfdr = accept(sockfdr,(struct sockaddr *) &cli_addr, &clilen);
    if (newsockfdr < 0)
    {
        printf("ERROR on accept\n");
        exit (1);
    }
    //close(sockfd);


    counter = 0;
    per = 0;
    good_packets = 0;
    bad_packets = 0;
    feedback_timer = timer_create();
    window_timer = timer_create();
    timer_tic(feedback_timer);
    timer_tic(window_timer);
}

// destructor
CE_CORNET3Drx::~CE_CORNET3Drx() {
    close(newsockfdr);
    close(sockfdr);
}

// execute function
void CE_CORNET3Drx::execute(ExtensibleCognitiveRadio *ECR){

int i = 0;

    // setup file descriptor to listen for data on TCP controller link.
    fd_set fds;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100;
    FD_ZERO(&fds);
    FD_SET(newsockfd, &fds);

    // if data is available, read it in
    if(select(newsockfd+1, &fds, NULL, NULL, &timeout)){
        // read the first byte which designates the message type
        i = recv(newsockfd, buffer, 8, 0);
    }

    if (i < 0)
    {
        perror("recv");
    }
    else if(i > 0)
    {
	    //If user closes CORNET3D, it sends a message starting with 9 to tell 
	    //the CE to shut down. Killing CRTS_controller (which runs on the same
	    //node as the transmitter) causes a graceful shutdown on both all nodes
	    if (buffer[7]=='1')
		    float rx_freq=140e6;
	    if (buffer[7]=='2')
		    float rx_freq=160e6;
	    if (buffer[7]=='3')
		    float rx_freq=500e6;
	    if (buffer[7]=='4')
		    float rx_freq=770e6;
	    if (buffer[7]=='5')
		    float rx_freq=1800e6;
	    if (buffer[7]=='6')
		    float rx_freq=1900e6;
	    if (buffer[7]=='7')
		    float rx_freq=3500e6;
	    ECR->set_rx_freq(tx_frq);
    }

    if(ECR->CE_metrics.CE_event != ExtensibleCognitiveRadio::TIMEOUT)
    {
        if(ECR->CE_metrics.control_valid && ECR->CE_metrics.payload_valid)
        {    
            good_packets++;
        }
        else if (!ECR->CE_metrics.control_valid
                || !ECR->CE_metrics.payload_valid)
        {
            bad_packets++;
        }
        if(timer_toc(feedback_timer) > 1.0)
        {
            per = (float)bad_packets/(bad_packets + good_packets);
            std::ostringstream ss;
            ss << per;
            std::string s_per(ss.str());
            write(newsockfdr,s_per.c_str(),s_per.size());
            timer_tic(feedback_timer);
        }
        if(timer_toc(window_timer) > 5.0)
        {
            good_packets = 0;
            bad_packets = 0;
            timer_tic(window_timer);
        }

    }


}

