#include <stdlib.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <string>
#include <time.h>
#include <uhd/utils/msg.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <errno.h>
#include <signal.h>
#include <complex>
#include <liquid/liquid.h>
#include "interferer_defined_constants.hpp"
#include "interferer.hpp"
#include "node_parameters.hpp"
#include "read_configs.hpp"

// global variables
int sig_terminate;


// ========================================================================
//  FUNCTION:  Receive_command_from_controller
// ========================================================================
void Receive_command_from_controller(int *TCP_controller, 
                                     Interferer * inter, 
                                     struct node_parameters *np)
  {
  // Listen to socket for message from controller
  char command_buffer[500+sizeof(struct node_parameters)];
  int rflag = recv(*TCP_controller, 
                   command_buffer, 
                   1+sizeof(struct node_parameters), 
                   0);
  int err = errno;
  if (rflag <= 0)
    {
    if ((err == EAGAIN) || (err == EWOULDBLOCK))
      {
      return;
      }
    else
      {
      close(*TCP_controller);
      printf("Socket failure\n");
      exit(1);
      }
    }

  // Parse command
  switch (command_buffer[0])
    {
    case 's': // settings for upcoming scenario
      printf("Received settings for scenario\n");
      // copy node_parameters
      memcpy(np ,&command_buffer[1], sizeof(node_parameters));
      print_node_parameters(np);
      // set interferer parameters
      inter->usrp_tx->set_tx_freq(np->tx_freq);
      inter->usrp_tx->set_tx_rate(4*np->tx_rate);
      inter->tx_rate = np->tx_rate;
      inter->usrp_tx->set_tx_gain(np->tx_gain);
      inter->interference_type = np->interference_type;
      inter->period = np->period;
      inter->duty_cycle = np->duty_cycle;
      break;

    case 't': // terminate program
      printf("Received termination command from controller\n");
      exit(1);
      break; 
    }
  }


// ========================================================================
//  FUNCTION:  uhd_quiet
// ========================================================================
void uhd_quiet(uhd::msg::type_t type, const std::string &msg){}


// ========================================================================
//  FUNCTION:  help_CRTS_interferer
// ========================================================================
void help_CRTS_interferer() 
  {
  printf("CRTS_interferer -- Start a cognitive radio interferer node. Only needs to be run explicitly when using CRTS_controller with -m option.\n");
  printf("                -- This program must be run from the main CRTS directory.\n");
  printf(" -h : Help.\n");
  printf(" -t : Run Time - Length of time this node will run. In seconds.\n");
  printf("      Default: 20.0 s\n");
  printf(" -a : IP Address of node running CRTS_controller.\n");
  printf("      Default: 192.168.1.56.\n");
  }


// ========================================================================
//  FUNCTION:  terminate
// ========================================================================
void terminate(int signum)
  {
  sig_terminate = 1;
  }


// ========================================================================
//  FUNCTION:  Fill Buffer for Transmission
// ========================================================================
void FillBufferForTransmission(
  unsigned int randomFlag,
  std::vector<std::complex<float> > &tx_buffer)
  {
  for(unsigned int i = 0; i < tx_buffer.size(); i++)
    {
    if (randomFlag == 1)
      {
      tx_buffer[i].real(0.5*(float)rand()/(float)RAND_MAX-0.25);
      tx_buffer[i].imag(0.5*(float)rand()/(float)RAND_MAX-0.25);
      }
    else
      {
      tx_buffer[i].real(0.5);
      tx_buffer[i].imag(0.5);
      }
    }
  }



// ========================================================================
//  FUNCTION:  Build GMSK Transmission
// ========================================================================
unsigned int BuildGMSKTransmission(
    std::vector<std::complex<float> > &tx_buffer,
    unsigned int samplesInBuffer)
  {
  // allocate memory for GMSK 
  gmskframegen gmsk_fg; 
  gmsk_fg = gmskframegen_create();

  // header and payload for frame generators
  unsigned char header[INTERFERER_HEADER_LENGTH]; 
  unsigned char payload[INTERFERER_PAYLOAD_LENGTH];
	
  // generate a random header
  for(int j = 0; j < INTERFERER_HEADER_LENGTH; j++)
    {
    header[j] = rand() & 0xff;
    }

  // generate a random payload
  for(int j=0; j < INTERFERER_PAYLOAD_LENGTH; j++)
    {
    payload[j] = rand() & 0xff;
    }

  // generate frame
  gmskframegen_assemble(gmsk_fg, 
                        header, 
                        payload, 
                        INTERFERER_PAYLOAD_LENGTH, 
                        LIQUID_CRC_NONE, 
                        LIQUID_FEC_NONE, 
                        LIQUID_FEC_NONE);
  
  unsigned int frameLen = gmskframegen_getframelen(gmsk_fg); 

  int frame_complete = 0;
  unsigned int bufferIndex = samplesInBuffer; 
  while(!frame_complete)
    {
    frame_complete = 
      gmskframegen_write_samples(gmsk_fg, 
                                 &tx_buffer[bufferIndex]);
    bufferIndex += GMSK_K_VALUE; 
    }

  return frameLen; 
  }

// ========================================================================
//  FUNCTION:  Build RRC Transmission
// ========================================================================

unsigned int BuildRRCTransmission(
    std::vector<std::complex<float> > &tx_buffer,
    unsigned int samplesInBuffer)

  {
  unsigned int samplesAddedToBuffer = 0; 

  unsigned int k = 0;
  unsigned int m = 0; 
  float beta = 0.0f;
  unsigned int h_len = 0.0f; 
  firfilt_crcf rrc_filt; 

  k = RRC_K_VALUE;
  m = RRC_M_VALUE;
  beta = RRC_BETA_VALUE; 
  h_len = 2*k*m+1;
  float h[h_len];
  liquid_firdes_rrcos(k, m, beta, 0.0, h);
  rrc_filt = firfilt_crcf_create(h, h_len);

  unsigned int tx_buffer_len = 10 * USRP_BUFFER_LENGTH; 

  unsigned int samps_to_transmit;
  samps_to_transmit = 
      ((tx_buffer.size() - samplesInBuffer <= tx_buffer_len) ?
       (tx_buffer.size() - samplesInBuffer) : 
       tx_buffer_len);

  std::complex<float> complex_symbol;

  for(unsigned int j=0; j < samps_to_transmit; j++)
    {
    samplesAddedToBuffer++;
    samplesInBuffer++; 
    // generate a random QPSK symbol until within a filter length of the end
    if((j%k == 0) && 
       (samplesInBuffer < tx_buffer.size()-2*h_len))
      {
      complex_symbol.real(0.5*(float)roundf((float)rand()/(float)RAND_MAX)-0.25);
      complex_symbol.imag(0.5*(float)roundf((float)rand()/(float)RAND_MAX)-0.25);
      }
    // zero insert to interpolate
    else
      {
      complex_symbol.real(0.0);
      complex_symbol.imag(0.0);
      }

    firfilt_crcf_push(rrc_filt, complex_symbol);
    firfilt_crcf_execute(rrc_filt, &tx_buffer[j]);
    }
  return samplesAddedToBuffer; 
  }

// ========================================================================
//  FUNCTION:  Build OFDM Transmission
// ========================================================================
/*
void BuildOFDMTransmission()
  {
  unsigned int num_subcarriers = 0; 
  unsigned int cp_len = 0;
  unsigned int taper_len = 0;
  unsigned char * p = NULL;
  int frame_complete = 0; 
  ofdmflexframegenprops_s fgprops;
  ofdmflexframegen ofdm_fg;

  num_subcarriers = 2*(unsigned int)(np.tx_rate/30e3);
  cp_len = OFDM_CP_LENGTH; 
  taper_len = OFDM_TAPER_LENGTH; 
  p = NULL;
  frame_complete = OFDM_FRAME_COMPLETE_INITIAL_STATE; 
  ofdmflexframegenprops_init_default(&fgprops);
  ofdm_fg = ofdmflexframegen_create(num_subcarriers, 
                                    cp_len, 
                                    taper_len, 
                                    p, 
                                    &fgprops);


  // header and payload for frame generators
  unsigned char header[INTERFERER_HEADER_LENGTH]; 
  unsigned char payload[INTERFERER_PAYLOAD_LENGTH];
	

  // generate frame
  if (frame_complete) 
    {
    ofdmflexframegen_assemble(ofdm_fg, 
                              header, 
                              payload, 
                              INTERFERER_PAYLOAD_LENGTH);
    }

  for(int j=0; j<floor(tx_buffer_len/(num_subcarriers+cp_len)); j++)
    {
    frame_complete = 
      ofdmflexframegen_writesymbol(ofdm_fg, 
                                   &tx_buffer[j*(num_subcarriers+cp_len)]);
    samps_to_transmit += num_subcarriers+cp_len;
    if(frame_complete) 
      {
      break;
      }
    }

  // reduce amplitude of signal to avoid clipping
  for(int j=0; j<samps_to_transmit; j++)
    {
    tx_buffer[j] *= 0.125f;
    }

  // update sample counter
  samp_count += samps_to_transmit;
  }
*/

// ========================================================================
//  FUNCTION:  Transmit Interference
// ========================================================================

void TransmitInterference(
  Interferer interfererObj, 
  std::vector<std::complex<float> > & tx_buffer,
  int samps_to_transmit
  )
  {
  int tx_samp_count = 0;//samps_to_transmit;	
  int usrp_samps = USRP_BUFFER_LENGTH; 

  while(tx_samp_count < samps_to_transmit)
    {
    // modifications for the last group of samples generated
    if(samps_to_transmit-tx_samp_count <= USRP_BUFFER_LENGTH)
      {
      usrp_samps = samps_to_transmit-tx_samp_count;
      }
		
    interfererObj.usrp_tx->get_device()->send(&tx_buffer[tx_samp_count], 
                                              usrp_samps,
                                              interfererObj.metadata_tx,
				              uhd::io_type_t::COMPLEX_FLOAT32,
                                              uhd::device::SEND_MODE_FULL_BUFF
				              //TODO: Should we set a timeout here?
                                              );
				
    // update number of tx samples remaining
    tx_samp_count += USRP_BUFFER_LENGTH;
    if (sig_terminate) 
      {
      break;
      }
    }// usrp transmit while loop

  }  


// ========================================================================
//  FUNCTION:  Perform Duty Cycle ON
// ========================================================================
void PerformDutyCycle_On( Interferer interfererObj,
                          node_parameters np,
                          unsigned int num_samples_for_on_cycle)
  {
  std::vector<std::complex<float> > tx_buffer(num_samples_for_on_cycle);
  unsigned int samplesInBuffer; 
  unsigned int samples_to_transmit; 

  printf(" --> Starting Interferer On Cycle\n");

  samplesInBuffer = 0; 
  unsigned int randomFlag = (np.interference_type == (AWGN)) ? 1 : 0; 

  switch(np.interference_type)
    {
    case(CW):
    case(CW_SWEEP): 
    case(AWGN):
        FillBufferForTransmission(randomFlag,
                                  tx_buffer); 
        samplesInBuffer = tx_buffer.size(); 
        samples_to_transmit = samplesInBuffer; 
	break;

    case(GMSK):
      while (samplesInBuffer < tx_buffer.size())
        { 
        samplesInBuffer += BuildGMSKTransmission(tx_buffer,
                                                 samplesInBuffer); 
        }
      samples_to_transmit = samplesInBuffer; 
      break; 
    case(RRC):
      while (samplesInBuffer < tx_buffer.size())
	 {
         samplesInBuffer += BuildRRCTransmission(tx_buffer,
                                                 samplesInBuffer); 
         }
       break; 
	  //        case(OFDM):
	  //          BuildOFDMTransmission(); 
	  //          break; 
     }// interference type switch
			
  TransmitInterference(interfererObj, tx_buffer, samples_to_transmit); 
  }



// ========================================================================
//  FUNCTION:  Perform Duty Cycle OFF
// ========================================================================
void PerformDutyCycle_Off(Interferer interfererObj,
                          node_parameters np, 
                          unsigned int num_samples_for_off_cycle)
  {
  unsigned int off_sample_counter;
  unsigned int numUsrpSamples;
  unsigned int samplesRemaining; 

  printf(" --> Starting Interferer Off Cycle\n");
  off_sample_counter = 0;	
  numUsrpSamples = USRP_BUFFER_LENGTH; 
  std::vector<std::complex<float> > 
      usrp_buffer_off(USRP_BUFFER_LENGTH); 

  while(off_sample_counter < num_samples_for_off_cycle)
      {

      // modifications for the last group of samples generated
      samplesRemaining = num_samples_for_off_cycle - off_sample_counter; 
      if(samplesRemaining < numUsrpSamples)
        {
        numUsrpSamples = samplesRemaining; 
	}

      interfererObj.usrp_tx->get_device()->send(&usrp_buffer_off[0], 
                                                numUsrpSamples,
                                                interfererObj.metadata_tx,
                                                uhd::io_type_t::COMPLEX_FLOAT32,
				                uhd::device::SEND_MODE_FULL_BUFF
                                                //TODO: Should we set a timeout here?
			                        );
				
      // update number of tx samples remaining
      off_sample_counter += numUsrpSamples; 
      if (sig_terminate) 
        {
        break;
        }
      } // end while samples off loop
  }



// ==========================================================================
// ==========================================================================
// ==========================================================================
//  MAIN PROGRAM
// ==========================================================================
// ==========================================================================
// ==========================================================================

int main(int argc, char ** argv)
  {

  // register signal handlers
  signal(SIGINT, terminate);
  signal(SIGQUIT, terminate);
  signal(SIGTERM, terminate);

  // set default values
  float run_time = DEFAULT_RUN_TIME;
  char * controller_ipaddr = (char*) DEFAULT_CONTROLLER_IP_ADDRESS;
  int TCP_controller = socket(AF_INET, SOCK_STREAM, 0);

  // validate TCP Controller 
  if (TCP_controller < 0)
    {
    printf("ERROR: Receiver Failed to Create Client Socket\n");
    exit(EXIT_FAILURE);
    }

  // get command line parameters
  int d;
  while((d = getopt(argc, argv, "ht:a:")) != EOF)
    {
    switch(d)
      {
      case 'h': help_CRTS_interferer();       return 0;
      case 't': run_time = atof(optarg);      break;
      case 'a': controller_ipaddr = optarg;   break;
      }
    }

  // Parameters for connecting to server
  struct sockaddr_in controller_addr;
  memset(&controller_addr, 0, sizeof(controller_addr));
  controller_addr.sin_family = AF_INET;
  controller_addr.sin_addr.s_addr = inet_addr(controller_ipaddr);
  controller_addr.sin_port = htons(4444);
	
  // Attempt to connect client socket to server
  int connect_status = connect(TCP_controller, 
                               (struct sockaddr*)&controller_addr, 
                               sizeof(controller_addr));
  if (connect_status)
    {
    printf("Failed to Connect to server.\n");
    exit(EXIT_FAILURE);
    }
  printf("Connected to server\n");
	
  uhd::msg::register_handler(&uhd_quiet);

  // Create node parameters struct and interferer object
  struct node_parameters np;
  Interferer interfererObj;

  // Set metadata for interferer
  interfererObj.metadata_tx.start_of_burst = false;
  interfererObj.metadata_tx.end_of_burst = false;
  interfererObj.metadata_tx.has_time_spec = false;

  // Read initial scenario info from controller
  Receive_command_from_controller(&TCP_controller, &interfererObj, &np);
  fcntl(TCP_controller, F_SETFL, O_NONBLOCK);

  int iterations = (int)(run_time/interfererObj.period);
  
  // for some interference types, transmit all of the time
  // by setting duty_cycle = 1.0
  switch(np.interference_type)
    {  
    case (CW_SWEEP):
      interfererObj.duty_cycle = 1.0; 
      break;
    }
  unsigned int num_samples_total = 
    (int)(interfererObj.period * 4 * interfererObj.tx_rate);
  unsigned int num_samples_on    = 
    (int)(num_samples_total * interfererObj.duty_cycle); 
  unsigned int num_samples_off   = 
    (int)(num_samples_total * (1.0 - interfererObj.duty_cycle));

  //printf("duty cycle: %f\n", inter.duty_cycle);
  //printf("samples on: %i\n", samps_on);
  //printf("samples off: %i\n", samples_off);

  // ================================================================
  // BEGIN: Main Service Loop 
  // ================================================================
  sig_terminate = 0;

  for(int i=0; i<iterations; i++)
    {
    Receive_command_from_controller(&TCP_controller, &interfererObj, &np);
    PerformDutyCycle_On(interfererObj,
                        np, 
                        num_samples_on); 
    PerformDutyCycle_Off(interfererObj,
                         np,  
                         num_samples_off); 
    if (sig_terminate) 
      {
      break;
      }
    } // end main "for" interation loop 
  // ================================================================
  // END: Main Service Loop 
  // ================================================================

  printf("Sending termination message to controller\n");
  char term_message = 't';
  write(TCP_controller, &term_message, 1);
  }

