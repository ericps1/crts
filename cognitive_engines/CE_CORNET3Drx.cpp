#include "../include/CE.hpp"
#include "../include/ECR.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    printf ("constructor");
    sockfdr = socket(AF_INET, SOCK_STREAM, 0);  // tcp socket
    if (sockfdr < 0)
        printf ("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portnor = 6018;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnor);
    if (bind(sockfdr, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf ("error on binding");
        //exit(1);
    }
    listen(sockfdr,5);
    clilen = sizeof(cli_addr);
    newsockfdr = accept(sockfdr,(struct sockaddr *) &cli_addr, &clilen);
    if (newsockfdr < 0)
    {
        printf("ERROR on accept");
        //exit (1);
    }
    //close(sockfd);
}


// destructor
CE_CORNET3Drx::~CE_CORNET3Drx() {}

// execute function
void CE_CORNET3Drx::execute(void * _args){
    // type cast pointer to cognitive radio object
    ExtensibleCognitiveRadio * ECR = (ExtensibleCognitiveRadio *) _args;
    int i=0,j=0,l=0,error=0;
    
    char a[256], b[256];
    if((ECR->CE_metrics.CE_event != ExtensibleCognitiveRadio::TIMEOUT) && ECR->CE_metrics.control_valid)
        error = 0;
    else
        error += 1;
   // itoa (error,buffer,10);
  // stringstream ss;
  // ss << error;
  // string str = ss.str();
if (error==0)
a[0]=0;
else
{
   for (i=error,j=0;i!=0;i=i/10,j++)
       a[j]=i%10;
}
a[j]= '\0';
 for (j=0;a[j]!='\0';j++,l++);
 for (j=0;a[j]!= '\0';j++,l--)
 b[l]=a[j];
    write(newsockfdr,b,10);
    close(newsockfdr);
    close(sockfdr);
}

