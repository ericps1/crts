#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<string.h>
#include<unistd.h>
#include<net/if.h>
//#include<linux/if.h>
#include<linux/if_tun.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/ioctl.h>
#include<sys/stat.h>
#include<sys/select.h>
#include<sys/time.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<errno.h>
#include<stdarg.h>
#include<pthread.h>
#include"TUN.hpp"

void * CR_tx_worker(void * _arg);

int main(){

    // Create TUN interface
    char tun_name[IFNAMSIZ];
    strcpy(tun_name, "tun0");
    int tunfd = tun_alloc(tun_name, IFF_TUN);

    // set IP, define default gateway, and bring up interface
    system("ip addr add 10.0.0.2/24 dev tun0");
    system("route add default gw 10.0.0.1 tun0");
    system("ip link set dev tun0 up");
    printf("Set interface to up\n");

    // Create thread that reads from TUN interface
    pthread_t tx_process;
    pthread_create(&tx_process, NULL, CR_tx_worker, (void*)&tunfd);

    // Define address structure for a socket
    std::string server_ip = "10.0.0.1";
    int port = 4444;
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server.sin_port = htons(port);

    // Create socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd<0){
	printf("Failed to create socket\n");
	exit(1);
    }

    // Message to be sent over socket
    char message[] = "Hello World\n";

    // Write to TUN interface
    while(true){
	sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&server, sizeof(server));
    }

    return 0;
}

void * CR_tx_worker(void * _arg){
    int tunfd = *(int*)_arg;
    int buffer_len = 1024;
    char buffer[buffer_len];

    printf("Entered tx worker thread\n");

    while(true){
	int nread = cread(tunfd, buffer, buffer_len);
	for(int i=32; i<nread; i++)
	    printf("%c", buffer[i]);
	printf("\n");
    }
    return NULL;
}
