#include "../include/CE.hpp"
#include "../include/ECR.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "CE_CORNET3Dtx.hpp"

#if 0
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

#define EVM_buff_len 3

/*class CE_CORNET3Dtx
  {
  public:
  CE_CORNET3Dtx();
  void execute(void * _args);
  ~CE_CORNET3Dtx();
  };*/
int newsockfd, sockfd, portno;
// constructor
CE_CORNET3Dtx::CE_CORNET3Dtx()
{

    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    printf ("constructor");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // tcp socket
    if (sockfd < 0) 
    {
        printf ("ERROR opening socket");
        // exit (1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 6008;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf ("error on binding");     
        //exit(1);
    }
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
    { 
        printf("ERROR on accept");
        //exit (1);
    }
    //close(sockfd);
}

// destructor
CE_CORNET3Dtx::~CE_CORNET3Dtx() {}

// execute function
void CE_CORNET3Dtx::execute(void * _args){
    // type cast pointer to cognitive radio object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;
    int i, timer;
    char buffer[256];
    //int newsockfd
    for (timer=0; timer<=10; timer++)
    {
        i= recv (newsockfd,buffer, 8, 0);
    }


    /*for (timer=0; timer<=100; timer++)
      {
      i= read(newsockfd,buffer,256);
      }*/
    printf ("\n buffer= %s ", buffer);     
    if (i < 0) 
    { 
        printf("ERROR on read");
        //exit (1);
    }

    if (buffer[0]=='0' && buffer[1]=='1'  )       
        ECR->set_tx_modulation(LIQUID_MODEM_OOK);
    if (buffer[0]=='0' && buffer[1]=='2'  )
        ECR->set_tx_modulation(LIQUID_MODEM_BPSK);
    if (buffer[0]=='0' && buffer[1]=='3'  )    
        ECR->set_tx_modulation(LIQUID_MODEM_QPSK);
    if (buffer[0]=='0' && buffer[1]=='4'  )   
        ECR->set_tx_modulation(LIQUID_MODEM_PSK8);
    if (buffer[0]=='0' && buffer[1]=='5'  )
        ECR->set_tx_modulation(LIQUID_MODEM_PSK16);
    if (buffer[0]=='0' && buffer[1]=='6'  )    
        ECR->set_tx_modulation(LIQUID_MODEM_PSK32);
    if (buffer[0]=='0' && buffer[1]=='7'  )   
        ECR->set_tx_modulation(LIQUID_MODEM_PSK64);
    if (buffer[0]=='0' && buffer[1]=='8'  ) 
        ECR->set_tx_modulation(LIQUID_MODEM_PSK128);
    if (buffer[0]=='0' && buffer[1]=='9'  ) 
       ECR->set_tx_modulation(LIQUID_MODEM_QAM8);
    if (buffer[0]=='0' && buffer[1]=='0'  )   
        ECR->set_tx_modulation(LIQUID_MODEM_QAM16);
    if (buffer[0]=='1' && buffer[1]=='1'  )     
        ECR->set_tx_modulation(LIQUID_MODEM_QAM32);
    if (buffer[0]=='1' && buffer[1]=='2'  )     
        ECR->set_tx_modulation(LIQUID_MODEM_QAM64);
    if (buffer[0]=='1' && buffer[1]=='3'  )         
        ECR->set_tx_modulation(LIQUID_MODEM_ASK2);
    if (buffer[0]=='1' && buffer[1]=='4'  )         
        ECR->set_tx_modulation(LIQUID_MODEM_ASK4);
    if (buffer[0]=='1' && buffer[1]=='5'  )         
        ECR->set_tx_modulation(LIQUID_MODEM_ASK8);
    if (buffer[0]=='1' && buffer[1]=='6'  )        
        ECR->set_tx_modulation(LIQUID_MODEM_ASK16);
    if (buffer[0]=='1' && buffer[1]=='7'  )         
        ECR->set_tx_crc (LIQUID_CRC_8);
    if (buffer[2]=='2'  )    
        ECR->set_tx_crc (LIQUID_CRC_16);
    if (buffer[2]=='3'  )   
        ECR->set_tx_crc (LIQUID_CRC_24);
    if (buffer[2]=='4'  )    
        ECR->set_tx_crc (LIQUID_CRC_32); 
    if (buffer[3]=='1'  )
        ECR->set_tx_fec0(LIQUID_FEC_HAMMING74);
    if (buffer[3]=='2'  )
        ECR->set_tx_fec0(LIQUID_FEC_HAMMING128);
    if (buffer[3]=='3')
        ECR->set_tx_fec0(LIQUID_FEC_GOLAY2412); 
    if (buffer[3]=='4')
        ECR->set_tx_fec0(LIQUID_FEC_SECDED2216);
    if (buffer[3]=='5')
        ECR->set_tx_fec0(LIQUID_FEC_SECDED3932);
    if (buffer[3]=='6')
        ECR->set_tx_fec0(LIQUID_FEC_SECDED7264); 
    if (buffer[4]=='1')
        ECR->set_tx_fec1(LIQUID_FEC_HAMMING74);
    if (buffer[4]=='2')
        ECR->set_tx_fec1(LIQUID_FEC_HAMMING128);
    if (buffer[4]=='3')
        ECR->set_tx_fec1(LIQUID_FEC_GOLAY2412); 
    if (buffer[4]=='4')
        ECR->set_tx_fec1(LIQUID_FEC_SECDED2216);
    if (buffer[4]=='5')
        ECR->set_tx_fec1(LIQUID_FEC_SECDED3932);
    if (buffer[4]=='6')    
        ECR->set_tx_fec1(LIQUID_FEC_SECDED7264);
  //  float tx_freq = buffer[6];
    //    ECR->set_tx_freq ( tx_freq);
    write(newsockfd,"I got your message",18);
    close(newsockfd);
    close(sockfd);
}

// custom function definitions

