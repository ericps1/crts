#include <stdio.h>
#include <liquid/liquid.h>
#include <limits.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SC_CORNET_3D.hpp"

// constructor
SC_CORNET_3D::SC_CORNET_3D() {
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
  old_fec0 = 12;
  old_fec1 = 1;
  old_freq = 770e6;
  old_bandwidth = 1e6;
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
void SC_CORNET_3D::execute(int node, char fb_type, void *_arg) {
  char msg[16];

  msg[0] = CRTS_MSG_FEEDBACK;
  msg[1] = node;
  msg[2] = fb_type;
  
  int arg_len = get_feedback_arg_len(fb_type);
  
  struct ExtensibleCognitiveRadio::rx_statistics* stats = (struct ExtensibleCognitiveRadio::rx_statistics*)_arg;
  float per = stats->avg_per;
  memcpy((void*) &msg[3], _arg, arg_len);

  std::string s_per = std::to_string(per);
  if(node == 0)
  {
      // forward feedback to CORNET 3D web server
      send(TCP_CORNET_3D, s_per.c_str(), sizeof(s_per), 0);
  }
  // forward commands from CORNET 3D webserver to node
  fd_set fds;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 100;
  FD_ZERO(&fds);
  FD_SET(TCP_CORNET_3D, &fds);

  if(select(TCP_CORNET_3D+1, &fds, NULL, NULL, &timeout)) 
  {
      int rlen = recv(TCP_CORNET_3D, msg, 11, 0);
      if (rlen < 0) 
      {
          printf("CORNET 3D socket failure\n");
          exit(1);
      }

      int mod;
      int crc;
      int fec0;
      int fec1;
      double freq;
      double bandwidth;

      //The same control code is sent to both nodes. For the tx node, we need to 
      //look at everything.
      //For the Rx node, we only need to look at freq and bandwidth since the 
      //other controls (mod, fec...) get sent in the header
      if(node == 1)
      {
          //Modulation Scheme encoded into msg[0] and msg[1]
          if (msg[0]=='0' && msg[1]=='1')       
              mod = LIQUID_MODEM_OOK;
          if (msg[0]=='0' && msg[1]=='2')
              mod = LIQUID_MODEM_BPSK;
          if (msg[0]=='0' && msg[1]=='3')    
              mod = LIQUID_MODEM_QPSK;
          if (msg[0]=='0' && msg[1]=='4')   
              mod = LIQUID_MODEM_PSK8;
          if (msg[0]=='0' && msg[1]=='5')
              mod = LIQUID_MODEM_PSK16;
          if (msg[0]=='0' && msg[1]=='6')    
              mod = LIQUID_MODEM_PSK32;
          if (msg[0]=='0' && msg[1]=='7')   
              mod = LIQUID_MODEM_PSK64;
          if (msg[0]=='0' && msg[1]=='8') 
              mod = LIQUID_MODEM_PSK128;
          if (msg[0]=='0' && msg[1]=='9') 
              mod = LIQUID_MODEM_QAM4;
          if (msg[0]=='1' && msg[1]=='0') 
              mod = LIQUID_MODEM_QAM8;
          if (msg[0]=='1' && msg[1]=='1')   
              mod = LIQUID_MODEM_QAM16;
          if (msg[0]=='1' && msg[1]=='2')     
              mod = LIQUID_MODEM_QAM32;
          if (msg[0]=='1' && msg[1]=='3')     
              mod = LIQUID_MODEM_QAM64;
          if (msg[0]=='1' && msg[1]=='4')         
              mod = LIQUID_MODEM_ASK2;
          if (msg[0]=='1' && msg[1]=='5')         
              mod = LIQUID_MODEM_ASK4;
          if (msg[0]=='1' && msg[1]=='6')         
              mod = LIQUID_MODEM_ASK8;
          if (msg[0]=='1' && msg[1]=='7')        
              mod = LIQUID_MODEM_ASK16;
          if (msg[0]=='1' && msg[1]=='8')        
              mod = LIQUID_MODEM_ASK32;
          if (msg[0]=='1' && msg[1]=='9')        
              mod = LIQUID_MODEM_ASK64;
          if (msg[0]=='2' && msg[1]=='0')        
              mod = LIQUID_MODEM_ASK128;

          if(mod != old_mod)
          {
              set_node_parameter(0, CRTS_TX_MOD, &mod);  
              old_mod = mod;
          }

          //Checksum encoded into msg[2]
          if (msg[2] == '1')
              crc = LIQUID_CRC_NONE;
          if (msg[2] == '2')
              crc = LIQUID_CRC_CHECKSUM;
          if (msg[2] =='3')    
              crc = LIQUID_CRC_8;
          if (msg[2] =='4')   
              crc = LIQUID_CRC_16;
          if (msg[2] =='5')    
              crc = LIQUID_CRC_24; 
          if (msg[2] =='6')    
              crc = LIQUID_CRC_32; 

          if(crc != old_crc)
          {
              set_node_parameter(0, CRTS_TX_CRC, &crc);
              old_crc = crc;
          }

          //Inner FEC encoded in msg[3] and msg[4]
          if (msg[3]=='0' && msg[4] == '1')
              fec0 = LIQUID_FEC_NONE;
          if (msg[3]=='0' && msg[4] == '2')
              fec0 = LIQUID_FEC_HAMMING74;
          if (msg[3]=='0' && msg[4] == '3')
              fec0 = LIQUID_FEC_HAMMING128; 
          if (msg[3]=='0' && msg[4] == '4')
              fec0 = LIQUID_FEC_GOLAY2412;
          if (msg[3]=='0' && msg[4] == '5')
              fec0 = LIQUID_FEC_SECDED2216;
          if (msg[3]=='0' && msg[4] == '6')
              fec0 = LIQUID_FEC_SECDED3932; 
          if (msg[3]=='0' && msg[4] == '7')
              fec0 = LIQUID_FEC_SECDED7264; 
          if (msg[3]=='0' && msg[4] == '8')
              fec0 = LIQUID_FEC_CONV_V27; 
          if (msg[3]=='0' && msg[4] == '9')
              fec0 = LIQUID_FEC_CONV_V29; 
          if (msg[3]=='1' && msg[4] == '0')
              fec0 = LIQUID_FEC_CONV_V39; 
          if (msg[3]=='1' && msg[4] == '1')
              fec0 = LIQUID_FEC_CONV_V615; 

          if(fec0 != old_fec0)
          {
              set_node_parameter(0, CRTS_TX_FEC0, &fec0);
              old_fec0 = fec0;
          }   

          //Outer FEC encoded in msg[5] and msg[6]
          if (msg[5]=='0' && msg[6] == '1')
              fec1 = LIQUID_FEC_NONE;
          if (msg[5]=='0' && msg[6] == '2')
              fec1 = LIQUID_FEC_HAMMING74;
          if (msg[5]=='0' && msg[6] == '3')
              fec1 = LIQUID_FEC_HAMMING128; 
          if (msg[5]=='0' && msg[6] == '4')
              fec1 = LIQUID_FEC_GOLAY2412;
          if (msg[5]=='0' && msg[6] == '5')
              fec1 = LIQUID_FEC_SECDED2216;
          if (msg[5]=='0' && msg[6] == '6')
              fec1 = LIQUID_FEC_SECDED3932; 
          if (msg[5]=='0' && msg[6] == '7')
              fec1 = LIQUID_FEC_SECDED7264; 
          if (msg[5]=='0' && msg[6] == '8')
              fec1 = LIQUID_FEC_CONV_V27; 
          if (msg[5]=='0' && msg[6] == '9')
              fec1 = LIQUID_FEC_CONV_V29; 
          if (msg[5]=='1' && msg[6] == '0')
              fec1 = LIQUID_FEC_CONV_V39; 
          if (msg[5]=='1' && msg[6] == '1')
              fec1 = LIQUID_FEC_CONV_V615; 

          if(fec1 != old_fec1)
          {
              set_node_parameter(0, CRTS_TX_FEC1, &fec1);
              old_fec1 = fec1;
          }

          //Frequency encoded in msg[7] 
          if (msg[7]=='1')
              freq = 140e6;
          if (msg[7]=='2')
              freq = 160e6;
          if (msg[7]=='3')
              freq = 500e6;
          if (msg[7]=='4')
              freq = 770e6;
          if (msg[7]=='5')
              freq = 1800e6;
          if (msg[7]=='6')
              freq = 1900e6;
          if (msg[7]=='7')
              freq = 3500e6;

          if(freq != old_freq)
          {
              set_node_parameter(0, CRTS_TX_FREQ, &freq);
              set_node_parameter(1, CRTS_RX_FREQ, &freq);
              old_freq = freq;
          }


          //Bandwidth encoded in msg[8] 
          if (msg[8] == '1')
              bandwidth = 200e3;
          if (msg[8] == '2')
              bandwidth = 500e3;
          if (msg[8] == '3')
              bandwidth = 1000e3;
          if (msg[8] == '4')
              bandwidth = 1500e3;
          if (msg[8] == '5')
              bandwidth = 2000e3;
          if (msg[8] == '6')
              bandwidth = 2500e3;
          if (msg[8] == '7')
              bandwidth = 5000e3;

          if(bandwidth != old_bandwidth)
          {
              set_node_parameter(0, CRTS_TX_RATE, &bandwidth);
              set_node_parameter(1, CRTS_RX_RATE, &bandwidth);
              old_bandwidth = bandwidth;
          }
      }
  }
}









