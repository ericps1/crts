#include "ECR.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "CE_CORNET3Dtx.hpp"
int newsockfd = 0, sockfd, portno;
// constructor
CE_CORNET3Dtx::CE_CORNET3Dtx()
{
    example_ce_metric = 125.0;
    counter = 0;
    flip = 1;

    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // tcp socket
    if (sockfd < 0) 
    {
        printf ("ERROR opening socket\n");
        // exit (1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 6008;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        printf ("error on binding\n");     
        exit(1);
    }
    printf("waiting for connection\n");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
    { 
        printf("ERROR on accept\n");
        exit (1);
    }

}

// destructor
CE_CORNET3Dtx::~CE_CORNET3Dtx() {
}

// execute function
void CE_CORNET3Dtx::execute(void * _args){
    // type cast pointer to cognitive radio object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;

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
        i = recv(newsockfd, buffer, 5, 0);
    }

    if (i < 0) 
    { 
        perror("recv");
    }
    else if(i > 0)
    {
        //Modulation Scheme encoded into buffer[0] and buffer[1]
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
            ECR->set_tx_modulation(LIQUID_MODEM_QAM4);
        if (buffer[0]=='1' && buffer[1]=='0'  ) 
            ECR->set_tx_modulation(LIQUID_MODEM_QAM8);
        if (buffer[0]=='1' && buffer[1]=='1'  )   
            ECR->set_tx_modulation(LIQUID_MODEM_QAM16);
        if (buffer[0]=='1' && buffer[1]=='2'  )     
            ECR->set_tx_modulation(LIQUID_MODEM_QAM32);
        if (buffer[0]=='1' && buffer[1]=='3'  )     
            ECR->set_tx_modulation(LIQUID_MODEM_QAM64);
        if (buffer[0]=='1' && buffer[1]=='4'  )         
            ECR->set_tx_modulation(LIQUID_MODEM_ASK2);
        if (buffer[0]=='1' && buffer[1]=='5'  )         
            ECR->set_tx_modulation(LIQUID_MODEM_ASK4);
        if (buffer[0]=='1' && buffer[1]=='6'  )         
            ECR->set_tx_modulation(LIQUID_MODEM_ASK8);
        if (buffer[0]=='1' && buffer[1]=='7'  )        
            ECR->set_tx_modulation(LIQUID_MODEM_ASK16);
        if (buffer[0]=='1' && buffer[1]=='8'  )        
            ECR->set_tx_modulation(LIQUID_MODEM_ASK32);
        if (buffer[0]=='1' && buffer[1]=='9'  )        
            ECR->set_tx_modulation(LIQUID_MODEM_ASK64);
        if (buffer[0]=='2' && buffer[1]=='0'  )        
            ECR->set_tx_modulation(LIQUID_MODEM_ASK128);

        //Checksum encoded into buffer[2]
        if (buffer[2] == '1')
            ECR->set_tx_crc (LIQUID_CRC_NONE);
        if (buffer[2] == '2')
            ECR->set_tx_crc (LIQUID_CRC_CHECKSUM);
        if (buffer[2]=='3'  )    
            ECR->set_tx_crc (LIQUID_CRC_8);
        if (buffer[2]=='4'  )   
            ECR->set_tx_crc (LIQUID_CRC_16);
        if (buffer[2]=='5'  )    
            ECR->set_tx_crc (LIQUID_CRC_24); 
        if (buffer[2]=='6'  )    
            ECR->set_tx_crc (LIQUID_CRC_32); 

        //Inner FEC encoded in buffer[3] and buffer[4]
        if (buffer[3]=='0' && buffer[4] == '1')
            ECR->set_tx_fec0(LIQUID_FEC_NONE);
        if (buffer[3]=='0' && buffer[4] == '2')
            ECR->set_tx_fec0(LIQUID_FEC_HAMMING74);
        if (buffer[3]=='0' && buffer[4] == '3')
            ECR->set_tx_fec0(LIQUID_FEC_HAMMING128); 
        if (buffer[3]=='0' && buffer[4] == '4')
            ECR->set_tx_fec0(LIQUID_FEC_GOLAY2412);
        if (buffer[3]=='0' && buffer[4] == '5')
            ECR->set_tx_fec0(LIQUID_FEC_SECDED2216);
        if (buffer[3]=='0' && buffer[4] == '6')
            ECR->set_tx_fec0(LIQUID_FEC_SECDED3932); 
        if (buffer[3]=='0' && buffer[4] == '7')
            ECR->set_tx_fec0(LIQUID_FEC_SECDED7264); 
        if (buffer[3]=='0' && buffer[4] == '8')
            ECR->set_tx_fec0(LIQUID_FEC_CONV_V27); 
        if (buffer[3]=='0' && buffer[4] == '9')
            ECR->set_tx_fec0(LIQUID_FEC_CONV_V29); 
        if (buffer[3]=='1' && buffer[4] == '0')
            ECR->set_tx_fec0(LIQUID_FEC_CONV_V39); 
        if (buffer[3]=='1' && buffer[4] == '1')
            ECR->set_tx_fec0(LIQUID_FEC_CONV_V615); 
        
        //Outer FEC encoded in buffer[5] and buffer[6]
        if (buffer[5]=='0' && buffer[6] == '1')
            ECR->set_tx_fec1(LIQUID_FEC_NONE);
        if (buffer[5]=='0' && buffer[6] == '2')
            ECR->set_tx_fec1(LIQUID_FEC_HAMMING74);
        if (buffer[5]=='0' && buffer[6] == '3')
            ECR->set_tx_fec1(LIQUID_FEC_HAMMING128); 
        if (buffer[5]=='0' && buffer[6] == '4')
            ECR->set_tx_fec1(LIQUID_FEC_GOLAY2412);
        if (buffer[5]=='0' && buffer[6] == '5')
            ECR->set_tx_fec1(LIQUID_FEC_SECDED2216);
        if (buffer[5]=='0' && buffer[6] == '6')
            ECR->set_tx_fec1(LIQUID_FEC_SECDED3932); 
        if (buffer[5]=='0' && buffer[6] == '7')
            ECR->set_tx_fec1(LIQUID_FEC_SECDED7264); 
        if (buffer[5]=='0' && buffer[6] == '8')
            ECR->set_tx_fec1(LIQUID_FEC_CONV_V27); 
        if (buffer[5]=='0' && buffer[6] == '9')
            ECR->set_tx_fec1(LIQUID_FEC_CONV_V29); 
        if (buffer[5]=='1' && buffer[6] == '0')
            ECR->set_tx_fec1(LIQUID_FEC_CONV_V39); 
        if (buffer[5]=='1' && buffer[6] == '1')
            ECR->set_tx_fec1(LIQUID_FEC_CONV_V615); 
    }
}



