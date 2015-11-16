#include "../include/CE.hpp"
#include "../include/ECR.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "CE_CORNET3Drx.hpp"
#include <sstream>
#if 0
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

#define EVM_buff_len 3

/*class CE_CORNET3Drx
  {
  public:
  CE_CORNET3Dtx();
  void execute(void * _args);
  ~CE_CORNET3Dtx();
  };*/
int newsockfdr, sockfdr, portnor;
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
        //exit(1);
    }
    printf("waiting on connection\n");
    listen(sockfdr,2);
    clilen = sizeof(cli_addr);
    newsockfdr = accept(sockfdr,(struct sockaddr *) &cli_addr, &clilen);
    if (newsockfdr < 0)
    {
        printf("ERROR on accept\n");
        //exit (1);
    }
    //close(sockfd);
    error = 0;
    last = 0;
    good_packets = 0;
    bad_packets = 0;
    per = 0;
}


// destructor
CE_CORNET3Drx::~CE_CORNET3Drx() {
    close(newsockfdr);
    close(sockfdr);

}

// execute function
void CE_CORNET3Drx::execute(void * _args){
    // type cast pointer to cognitive radio object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;

    if(ECR->CE_metrics.CE_event != ExtensibleCognitiveRadio::TIMEOUT)
    {
        if(ECR->CE_metrics.control_valid && ECR->CE_metrics.payload_valid)
        {    
            error = 0;
            good_packets++;
        }
        else if (!ECR->CE_metrics.control_valid || !ECR->CE_metrics.payload_valid)
        {
            error += 1;
            bad_packets++;
        }
        last++;
        if(last % 10 == 0)bad_packets++;	
        per = (float)bad_packets/(bad_packets + good_packets);
        std::ostringstream ss;
        ss << per;
        std::string s_per(ss.str());
        printf("sper: %s\n", s_per.c_str());
        write(newsockfdr,s_per.c_str(),s_per.size());
        //   char data_to_send[4];
        //memcpy(&data_to_send, &bad_packets, sizeof(error));
        //send(newsockfdr, (const char*)data_to_send, 4, 0);
    }
    else
        printf("timeout\n");

}

