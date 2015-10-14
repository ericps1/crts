#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
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
#include <fstream>
#include "interferer_defined_constants.hpp"
#include "interferer.hpp"
#include "node_parameters.hpp"
#include "read_configs.hpp"
#include "timer.h"

// global variables
time_t start_time_s;
int    sig_terminate;
float  currentTxFreq; 
float  freqIncrement; 
int    freqCoeff; 
int    freqWidth; 
int    TCP_controller; 
int    log_tx_parameters_flag;
char   tx_log_file[30];

// ========================================================================
//  FUNCTION:  Receive_command_from_controller
// ========================================================================
static inline void Receive_command_from_controller(Interferer * inter, 
                                                   struct node_parameters *np)
  {
  // Listen to socket for message from controller
  char command_buffer[500+sizeof(struct node_parameters)];
  int rflag = recv(TCP_controller, 
                   command_buffer, 
                   1+sizeof(time_t)+sizeof(struct node_parameters), 
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
      close(TCP_controller);
      printf("Socket failure\n");
      exit(1);
      }
    }

  // Parse command
  switch (command_buffer[0])
    {
    case 's': // settings for upcoming scenario
      printf("Received settings for scenario\n");
      
      // copy start time
      memcpy((void*)&start_time_s ,&command_buffer[1], sizeof(time_t));
            
      // copy node_parameters
      memcpy(np ,&command_buffer[1+sizeof(time_t)], sizeof(node_parameters));
      print_node_parameters(np);
      
      // set interferer parameters
      currentTxFreq = np->tx_freq;
      if (np->tx_freq_hop_type == (ALTERNATING))
        {
        currentTxFreq = np->tx_freq_hop_min; 
        }
      freqIncrement = +2.5e5; 
      freqCoeff = +1; 

      inter->usrp_tx->set_tx_freq(currentTxFreq);
      inter->usrp_tx->set_tx_rate(np->tx_rate);
      inter->tx_rate = np->tx_rate;
      inter->usrp_tx->set_tx_gain(np->tx_gain);
      inter->interference_type = np->interference_type;
      inter->period = np->period;
      inter->duty_cycle = np->duty_cycle;

      // set freq hopping parameters
      inter->tx_freq_hop_type       = np->tx_freq_hop_type;
      inter->tx_freq_hop_min        = np->tx_freq_hop_min; 
      inter->tx_freq_hop_max        = np->tx_freq_hop_max; 
      inter->tx_freq_hop_dwell_time = np->tx_freq_hop_dwell_time; 
      inter->tx_freq_hop_increment  = np->tx_freq_hop_increment;
      if (inter->tx_freq_hop_increment > 0.0)
      {
          freqIncrement = inter->tx_freq_hop_increment; 
      }
      freqWidth = floor(np->tx_freq_hop_max - np->tx_freq_hop_min); 

      // set gmsk parameters 
      inter->gmsk_header_length = np->gmsk_header_length; 
      inter->gmsk_payload_length = np->gmsk_payload_length; 
      inter->gmsk_bandwidth = np->gmsk_bandwidth; 

      log_tx_parameters_flag = np->log_tx_parameters;
      strcpy(tx_log_file, np->tx_log_file);

      // open tx log file to delete any current contents
      if(log_tx_parameters_flag){
        std::ofstream log_file;
        char log_file_name[50];
        strcpy(log_file_name, "./logs/");
        strcat(log_file_name, tx_log_file);
        log_file.open(log_file_name, std::ofstream::out | std::ofstream::trunc);
        if (log_file.is_open())
        {
          log_file.close();
        }
        else
        {
          std::cout << "Error opening log file: " << log_file_name << std::endl;
        }
      }

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
// 
//  Based upon liquid-usrp gmskframe_tx.cc
// ========================================================================
unsigned int BuildGMSKTransmission(
  std::vector<std::complex<float> > &tx_buffer,
  Interferer InterfererObj)
  {
       printf("\n"); 
       printf("============================================== \n"); 
       printf("--> BuildGMSKTransmission \n"); 
  std::vector<std::complex<float> > tempBuffer; 

  // Allocate and set GMSK Variable defaults
  float gmskMinBandwidth = 0.25f * (DAC_RATE / 256.0); 
  float gmskMaxBandwidth = 0.25f * (DAC_RATE / 4.0); 
  float gmskBandWidth = 100e3;        // [Hz]
  float gmskTxGainDb = -3.0f;         // Software Gain -3dB by default 
  unsigned int gmskHeaderLength = INTERFERER_HEADER_LENGTH; 
  unsigned int gmskPayloadLength = INTERFERER_PAYLOAD_LENGTH; 
  crc_scheme gmskCrcScheme = LIQUID_CRC_16; 
  modulation_scheme gmskModulationScheme = LIQUID_MODEM_QPSK;
  fec_scheme gmskFecSchemeInner = LIQUID_FEC_NONE; 
  fec_scheme gmskFecSchemeOuter = LIQUID_FEC_HAMMING74; 

  // allocate memory for GMSK Frame Generator 
  gmskframegen gmsk_fg; 
  gmsk_fg = gmskframegen_create();

  // Update any exposed values from node parameters
  gmskHeaderLength  = InterfererObj.gmsk_header_length; 
  gmskPayloadLength = InterfererObj.gmsk_payload_length;  
  gmskBandWidth = InterfererObj.gmsk_bandwidth; 
  gmskTxGainDb = InterfererObj.tx_gain_soft; 

  // assert values for GMSK transmission 
  if (gmskBandWidth > gmskMaxBandwidth) 
    {
    fprintf(stderr,"error: Interferer GMSK maximum bandwidth exceeded (%8.4f MHz)\n", gmskMaxBandwidth*1e-6);
    return 1;
    } 
  else if (gmskBandWidth < gmskMinBandwidth) 
    {
    fprintf(stderr,"error: Interferer GMSK minimum bandwidth exceeded (%8.4f kHz)\n", gmskMinBandwidth*1e-3);
    return 1;
    }
  else if (gmskPayloadLength > (1<<16)) 
    {
    fprintf(stderr,"error: Interferer GMSK maximum payload length exceeded: %u > %u\n", gmskPayloadLength, 1<<16);
    return 1;
    } 
  else if (gmskFecSchemeInner == LIQUID_FEC_UNKNOWN || 
           gmskFecSchemeOuter == LIQUID_FEC_UNKNOWN) 
    {
    fprintf(stderr,"error: unsupported FEC scheme\n");
    return 1;
    } 
  else if (gmskModulationScheme == LIQUID_MODEM_UNKNOWN) 
    {
    fprintf(stderr,"error: unsupported modulation scheme\n");
    return 0;
    }

  // calculate tx rate
  double tx_rate = 4.0 * gmskBandWidth; 
    printf("tx rate %e \n", tx_rate); 
  unsigned int interp_rate = (unsigned int)(DAC_RATE / tx_rate); 
  interp_rate = (interp_rate >> 2) << 2; 
  interp_rate += 4; 
  double usrp_tx_rate = DAC_RATE / (double)interp_rate; 
  InterfererObj.usrp_tx->set_tx_rate(usrp_tx_rate);
  usrp_tx_rate = InterfererObj.usrp_tx->get_tx_rate(); 

  double tx_resamp_rate = usrp_tx_rate / tx_rate; 
   printf("resample rate for arbitrary resampler: %f \n", tx_resamp_rate); 

  // half-band resampler
  resamp2_crcf interp = resamp2_crcf_create(7,0.0f,40.0f);

  // add arbitrary resampling component
  resamp_crcf resamp = resamp_crcf_create(tx_resamp_rate,7,0.4f,60.0f,64);
  resamp_crcf_setrate(resamp, tx_resamp_rate);

  // header and payload for frame generators
  unsigned char header[gmskHeaderLength]; 
  unsigned char payload[gmskPayloadLength];
    
  // generate a random header
  for(unsigned int j = 0; j < gmskHeaderLength; j++)
    {
    header[j] = rand() & 0xff;
    }

  // generate a random payload
  for(unsigned int j=0; j < gmskPayloadLength; j++)
    {
    payload[j] = rand() & 0xff;
    }

  // generate frame
  gmskframegen_assemble(gmsk_fg, 
                        header, 
                        payload, 
                        gmskPayloadLength, 
            gmskCrcScheme, 
            gmskFecSchemeInner, 
            gmskFecSchemeOuter); 
  
  unsigned int frameLen = gmskframegen_getframelen(gmsk_fg); 

    printf("header length:   %d \n", gmskHeaderLength); 
    printf("payload length:  %d \n", gmskPayloadLength); 
    printf("bandwidth:       %-.2e \n", gmskBandWidth); 
    printf("frame length:    %d \n", frameLen); 

  tempBuffer.resize(frameLen + 1); 
  int frame_complete = 0;

  // set up framing buffers 
  unsigned int k = 2; 
  std::complex<float> buffer[k];
  std::complex<float> buffer_interp[2*k]; 
  std::complex<float> buffer_resamp[8*k];
  //  std::vector<std::complex<float> > buff(5000); 
  unsigned int tx_buffer_samples = 0; 

  // calculate soft gain
  float g = powf(10.0f, gmskTxGainDb/20.0f); 

  // generate frame
  while (!frame_complete) 
    {
    // generate k samples
    frame_complete = gmskframegen_write_samples(gmsk_fg, buffer);

    // interpolate by 2
    for (unsigned int j=0; j<k; j++)
      {
      resamp2_crcf_interp_execute(interp, buffer[j], &buffer_interp[2*j]);
      }

    // resample
    unsigned int nw;
    unsigned int n=0;
    for (unsigned int j=0; j<2*k; j++) 
      {
      resamp_crcf_execute(resamp, buffer_interp[j], &buffer_resamp[n], &nw);
      n += nw;
      }

    // push onto buffer with software gain
    for (unsigned int j=0; j<n; j++)
      {
        tx_buffer[tx_buffer_samples++] = g * buffer_resamp[j]; 
      }
    }

  //    for (unsigned int j = 0; j < tx_buffer_samples; j++)
  //     {
  //     tx_buffer[j] = buff[j]; 
  //      }

    printf("frame interpolated and resampled \n"); 
    printf("final buffer length for frame:  %d \n", tx_buffer_samples); 
    printf("============================================== \n"); 
  return tx_buffer_samples; 
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
  unsigned char * subcarrierAlloc = NULL;
  int frame_complete = 0; 
  ofdmflexframegenprops_s fgprops;
  ofdmflexframegen ofdm_fg;

  num_subcarriers = 2*(unsigned int)(np.tx_rate/30e3);
  cp_len = OFDM_CP_LENGTH; 
  taper_len = OFDM_TAPER_LENGTH; 
  subcarrierAlloc = NULL;
  frame_complete = OFDM_FRAME_COMPLETE_INITIAL_STATE; 
  ofdmflexframegenprops_init_default(&fgprops);
  ofdm_fg = ofdmflexframegen_create(num_subcarriers, 
                                    cp_len, 
                                    taper_len, 
                                    subcarrierAlloc, 
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
//  FUNCTION:  Log transmission parameters
// ========================================================================
void log_tx_parameters(){

    // update current time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    // create string of actual file location
    char file_name[50];
    strcpy(file_name, "./logs/");
    strcat(file_name, tx_log_file);

    // open file, append parameters, and close
    std::ofstream log_file;
    log_file.open(file_name, std::ofstream::out|std::ofstream::binary|std::ofstream::app);
    if (log_file.is_open())
    {
        log_file.write((char*)&tv, sizeof(tv));
        log_file.write((char*)&currentTxFreq, sizeof(currentTxFreq));
    }
    else
    {
        std::cerr << "Error opening log file:" << file_name << std::endl;
    }
    log_file.close();
}

// ========================================================================
//  FUNCTION:  Transmit Interference
// ========================================================================

void TransmitInterference(
  Interferer interfererObj, 
  std::vector<std::complex<float> > & tx_buffer,
  int samplesInBuffer,
  node_parameters  np
  )
  {
  int tx_samp_count = 0;//samps_to_transmit;    
  int usrp_samps = USRP_BUFFER_LENGTH; 

  if(log_tx_parameters_flag)
      log_tx_parameters();
  
  while(tx_samp_count < samplesInBuffer) 
    {
    // modifications for the last group of samples generated
    if(samplesInBuffer - tx_samp_count <= USRP_BUFFER_LENGTH)
      {
      usrp_samps = samplesInBuffer - tx_samp_count;
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
    Receive_command_from_controller(&interfererObj, &np);
    if (sig_terminate) 
      {
      break;
      }
    }// usrp transmit while loop

  }  


// ========================================================================
//  FUNCTION:  Set Tx Freq for Frequency Hopping 
// ========================================================================
void ChangeFrequency(Interferer interfererObj)
  {
  
  switch (interfererObj.tx_freq_hop_type)
    {
    case (ALTERNATING):
      if (currentTxFreq == interfererObj.tx_freq_hop_max) 
    {
        currentTxFreq = interfererObj.tx_freq_hop_min; 
    }
      else
    {
        currentTxFreq = interfererObj.tx_freq_hop_max; 
    }
      break;
    case (SWEEP):
      currentTxFreq += (freqIncrement * freqCoeff); 
      if ((currentTxFreq > interfererObj.tx_freq_hop_max) ||
          (currentTxFreq < interfererObj.tx_freq_hop_min))
        {
        freqCoeff = freqCoeff * -1;  
        currentTxFreq = currentTxFreq + (2 * freqIncrement * freqCoeff); 
        }
      break; 
    case (RANDOM):
      currentTxFreq = rand() % freqWidth + interfererObj.tx_freq_hop_min;
      break;
    }
  printf("Set transmit frequency to %f\n", currentTxFreq);
  interfererObj.usrp_tx->set_tx_freq(currentTxFreq);
  }


// ========================================================================
//  FUNCTION:  Perform Duty Cycle ON
// ========================================================================
void PerformDutyCycle_On( Interferer interfererObj,
              node_parameters np,
                          float time_onCycle)
  {
  std::vector<std::complex<float> > tx_buffer(TX_BUFFER_LENGTH);
  unsigned int samplesInBuffer = 0; 
  unsigned int randomFlag = 
    (interfererObj.interference_type == (AWGN)) ? 1 : 0; 
  timer onTimer = timer_create(); 
  timer dwellTimer = timer_create(); 
  timer_tic(onTimer); 
  timer_tic(dwellTimer); 

  while (timer_toc(onTimer) < time_onCycle)
    {
    // determine if we need to freq hop 
    if ((interfererObj.tx_freq_hop_type != (NONE)) && 
        (timer_toc(dwellTimer) >= interfererObj.tx_freq_hop_dwell_time))
      {
      ChangeFrequency(interfererObj); 
      usleep(100); 
      timer_tic(dwellTimer); 
      } 

    // Generate One Frame of Data to Transmit 
    switch(interfererObj.interference_type)
      {
      case(CW):
      case(AWGN):
        FillBufferForTransmission(randomFlag,
                                  tx_buffer); 
        samplesInBuffer = tx_buffer.size(); 
    break;

      case(GMSK):
      samplesInBuffer = BuildGMSKTransmission(tx_buffer,
                                                  interfererObj); 
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

    Receive_command_from_controller(&interfererObj, &np);
    if (sig_terminate) 
      {
      break;
      }
    TransmitInterference(interfererObj, tx_buffer, samplesInBuffer, np); 
    Receive_command_from_controller(&interfererObj, &np);
    if (sig_terminate) 
      {
      break;
      }

    }
  timer_destroy(onTimer); 
  timer_destroy(dwellTimer); 
  }


// ========================================================================
//  FUNCTION:  Perform Duty Cycle OFF
// ========================================================================

void PerformDutyCycle_Off(Interferer interfererObj, 
                          node_parameters np,
                          float time_offCycle)
  {
  timer offTimer = timer_create(); 
  timer_tic(offTimer); 
  while (timer_toc(offTimer) < time_offCycle)
      {
      usleep(100); 
      Receive_command_from_controller(&interfererObj, &np);
      if (sig_terminate) 
        {
        break;
        }
      }
  timer_destroy(offTimer); 
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
  time_t run_time = DEFAULT_RUN_TIME;
  char* controller_ipaddr = (char*) DEFAULT_CONTROLLER_IP_ADDRESS;
  TCP_controller = socket(AF_INET, SOCK_STREAM, 0);

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
  
  uhd::msg::register_handler(&uhd_quiet);

  // Create node parameters struct and interferer object
  struct node_parameters np;
  Interferer interfererObj;

  // Set metadata for interferer
  interfererObj.metadata_tx.start_of_burst = false;
  interfererObj.metadata_tx.end_of_burst = false;
  interfererObj.metadata_tx.has_time_spec = false;

  // Read initial scenario info from controller
  Receive_command_from_controller(&interfererObj, &np);
  fcntl(TCP_controller, F_SETFL, O_NONBLOCK);

  // for some interference types, transmit all of the time
  // by setting duty_cycle = 1.0
  switch(np.tx_freq_hop_type)
    {  
    case (ALTERNATING):
    case (SWEEP):
    case (RANDOM):
      if (interfererObj.period != run_time)
        {
        printf("NOTICE:  Config Override, setting period to run_time \n"); 
        interfererObj.period = run_time; 
        }
      if (interfererObj.duty_cycle != 1.0)
    {  
        printf("NOTICE:  Config Override, setting duty_cycle to 1.0 \n"); 
        interfererObj.duty_cycle = 1.0; 
    }
      break;
    }

  // ================================================================
  // BEGIN: Main Service Loop 
  // ================================================================
  sig_terminate = 0;
  float time_onCycle = (interfererObj.period * interfererObj.duty_cycle); 
  float time_offCycle = (interfererObj.period * (1 - interfererObj.duty_cycle)); 

  // wait for start time and calculate stop time
  struct timeval tv;
  time_t time_s;
  time_t stop_time_s = start_time_s + run_time;
  while(1){
    gettimeofday(&tv, NULL);
    time_s = tv.tv_sec;
    if(time_s >= start_time_s)
      break;
  }
  
  while (time_s < stop_time_s)
    {
    Receive_command_from_controller(&interfererObj, &np);
    if (sig_terminate) 
      {
      break;
      }
    PerformDutyCycle_On(interfererObj,
                        np, 
                        time_onCycle); 
    PerformDutyCycle_Off(interfererObj,
                         np,
                         time_offCycle); 
    if (sig_terminate) 
      {
      break;
      }
    
    // update current time
    gettimeofday(&tv, NULL);
    time_s = tv.tv_sec;

    } // end main "for" interation loop 
  // ================================================================
  // END: Main Service Loop 
  // ================================================================

  printf("Sending termination message to controller\n");
  char term_message = 't';
  write(TCP_controller, &term_message, 1);
  }


