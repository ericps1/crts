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

#define DEBUG 0
#if DEBUG == 1
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

// global variables
time_t stop_time_s;
int sig_terminate;
float currentTxFreq;
float freqIncrement;
int freqCoeff;
int freqWidth;
int TCP_controller;
int log_phy_tx_flag;
char phy_tx_log_file[30];

timer onTimer;
timer dwellTimer;

resamp2_crcf interp;
gmskframegen gmsk_fg;

firfilt_crcf rrc_filt;

ofdmflexframegenprops_s fgprops;
ofdmflexframegen ofdm_fg;

// files to debug dsp
FILE *GMSK_log;
FILE *GMSK2_log;
// FILE * GMSKA_log;

// ========================================================================
//  FUNCTION:  Receive_command_from_controller
// ========================================================================
static inline void
Receive_command_from_controller(Interferer *Int, struct node_parameters *np,
                                struct scenario_parameters *sp) {
  fd_set fds;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 1;
  FD_ZERO(&fds);
  FD_SET(TCP_controller, &fds);

  // Listen to socket for message from controller
  if (select(TCP_controller + 1, &fds, NULL, NULL, &timeout)) {
    char command_buffer[1 + sizeof(struct scenario_parameters) +
                        sizeof(struct node_parameters)];
    int rflag = recv(TCP_controller, command_buffer,
                     1 + sizeof(struct scenario_parameters) +
                         sizeof(struct node_parameters),
                     0);
    int err = errno;
    if (rflag <= 0) {
      if ((err == EAGAIN) || (err == EWOULDBLOCK)) {
        return;
      } else {
        close(TCP_controller);
        printf("Socket failure\n");
        exit(1);
      }
    }

    // Parse command
    switch (command_buffer[0]) {
    case scenario_params_msg: // settings for upcoming scenario
      dprintf("Received settings for scenario\n");

      // copy scenario parameters
      memcpy(sp, &command_buffer[1], sizeof(struct scenario_parameters));

      // copy node_parameters
      memcpy(np, &command_buffer[1 + sizeof(struct scenario_parameters)],
             sizeof(struct node_parameters));
      print_node_parameters(np);

      // set interferer parameters
      currentTxFreq = np->tx_freq;
      if (np->tx_freq_hop_type == (ALTERNATING)) {
        currentTxFreq = np->tx_freq_hop_min;
      }
      freqIncrement = +2.5e5;
      freqCoeff = +1;

      // set general interference parameters
      Int->interference_type = np->interference_type;
      Int->period = np->period;
      Int->duty_cycle = np->duty_cycle;
      Int->tx_rate = np->tx_rate;
      Int->tx_gain_soft = np->tx_gain_soft;

      // set USRP settings
      Int->usrp_tx->set_tx_freq(currentTxFreq);
      Int->usrp_tx->set_tx_rate(np->tx_rate);
      Int->usrp_tx->set_tx_gain(np->tx_gain);

      // set freq hopping parameters
      Int->tx_freq_hop_type = np->tx_freq_hop_type;
      Int->tx_freq_hop_min = np->tx_freq_hop_min;
      Int->tx_freq_hop_max = np->tx_freq_hop_max;
      Int->tx_freq_hop_dwell_time = np->tx_freq_hop_dwell_time;
      Int->tx_freq_hop_increment = np->tx_freq_hop_increment;
      if (Int->tx_freq_hop_increment > 0.0) {
        freqIncrement = Int->tx_freq_hop_increment;
      }
      freqWidth = floor(np->tx_freq_hop_max - np->tx_freq_hop_min);

      log_phy_tx_flag = np->log_phy_tx;
      strcpy(phy_tx_log_file, np->phy_tx_log_file);

      // open tx log file to delete any current contents
      if (log_phy_tx_flag) {
        std::ofstream log_file;
        char log_file_name[50];
        strcpy(log_file_name, "./logs/");
        strcat(log_file_name, phy_tx_log_file);
        log_file.open(log_file_name, std::ofstream::out | std::ofstream::trunc);
        if (log_file.is_open()) {
          log_file.close();
        } else {
          std::cout << "Error opening log file: " << log_file_name << std::endl;
        }
      }

      break;

    case manual_start_msg: // updated start time (used for manual mode)
      dprintf("Received an updated start time\n");
      memcpy(&sp->start_time_s, &command_buffer[1], sizeof(time_t));
      stop_time_s = sp->start_time_s + sp->runTime;
      break;
    case terminate_msg: // terminate program
      dprintf("Received termination command from controller\n");
      exit(1);
      break;
    }
  }
}

// ========================================================================
//  FUNCTION:  uhd_quiet
// ========================================================================
void uhd_quiet(uhd::msg::type_t type, const std::string &msg) {}

// ========================================================================
//  FUNCTION:  help_CRTS_interferer
// ========================================================================
void help_CRTS_interferer() {
  printf("CRTS_interferer -- Start a cognitive radio interferer node. Only "
         "needs to be run explicitly when using CRTS_controller with -m "
         "option.\n");
  printf("                -- This program must be run from the main CRTS "
         "directory.\n");
  printf(" -h : Help.\n");
  printf(" -t : Run Time - Length of time this node will run. In seconds.\n");
  printf("      Default: 20.0 s\n");
  printf(" -a : IP Address of node running CRTS_controller.\n");
  printf("      Default: 192.168.1.56.\n");
}

// ========================================================================
//  FUNCTION:  terminate
// ========================================================================
void terminate(int signum) { sig_terminate = 1; }

// ========================================================================
//  FUNCTION:  Fill Buffer for Transmission
// ========================================================================
void FillBufferForTransmission(unsigned int randomFlag,
                               std::vector<std::complex<float>> &tx_buffer) {
  for (unsigned int i = 0; i < tx_buffer.size(); i++) {
    if (randomFlag == 1) {
      tx_buffer[i].real(0.5 * (float)rand() / (float)RAND_MAX - 0.25);
      tx_buffer[i].imag(0.5 * (float)rand() / (float)RAND_MAX - 0.25);
    } else {
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
unsigned int BuildGMSKTransmission(std::vector<std::complex<float>> &tx_buffer,
                                   Interferer Int) {
  crc_scheme gmskCrcScheme = LIQUID_CRC_16;
  fec_scheme gmskFecSchemeInner = LIQUID_FEC_NONE;
  fec_scheme gmskFecSchemeOuter = LIQUID_FEC_HAMMING74;

  // header and payload for frame generators
  unsigned char header[GMSK_HEADER_LENGTH];
  unsigned char payload[GMSK_PAYLOAD_LENGTH];

  // generate a random header
  for (unsigned int j = 0; j < GMSK_HEADER_LENGTH; j++) {
    header[j] = rand() & 0xff;
  }

  // generate a random payload
  for (unsigned int j = 0; j < GMSK_PAYLOAD_LENGTH; j++) {
    payload[j] = rand() & 0xff;
  }

  // generate frame
  int plen = GMSK_PAYLOAD_LENGTH;
  gmskframegen_assemble(gmsk_fg, header, payload, plen, gmskCrcScheme,
                        gmskFecSchemeInner, gmskFecSchemeOuter);

  // set up framing buffers
  unsigned int k = 2;
  std::complex<float> buffer[k];
  std::complex<float> buffer_interp[2 * k];
  // std::complex<float> buffer_resamp[8*k];
  unsigned int tx_buffer_samples = 0;

  // calculate soft gain
  float g = powf(10.0f, Int.tx_gain_soft / 20.0f);

  int frame_complete = 0;

  // generate frame
  while (!frame_complete) {

    // generate k samples
    frame_complete = gmskframegen_write_samples(gmsk_fg, buffer);

    // interpolate by 2
    for (unsigned int j = 0; j < k; j++) {
      resamp2_crcf_interp_execute(interp, buffer[j], &buffer_interp[2 * j]);
    }

    // resample
    // unsigned int nw;
    // unsigned int n=0;
    for (unsigned int j = 0; j < 2 * k; j++) {

      tx_buffer[tx_buffer_samples++] = g * buffer_interp[j];

      // resamp_crcf_execute(resamp, buffer_interp[j], &buffer_resamp[n], &nw);
      // n += nw;
    }

    // push onto buffer with software gain
    /*for (unsigned int j=0; j<n; j++)
      {
    //tx_buffer[tx_buffer_samples++] = g * buffer_resamp[j];
    tx_buffer[tx_buffer_samples++] = g * buffer_resamp[j];
    }*/
  }

  // Define zero-padding for the frame. This is important to allow the
  // resampler filters to go back to a relaxed state
  unsigned int padding = 6;

  for (unsigned int i = 0; i < padding; i++) {
    resamp2_crcf_interp_execute(interp, 0.0f, &buffer_interp[2 * i]);
    for (unsigned int j = 0; j < 2; j++) {
      tx_buffer[tx_buffer_samples++] = buffer_interp[j];
    }
  }

  return tx_buffer_samples;
}

// ========================================================================
//  FUNCTION:  Build RRC Transmission
// ========================================================================

unsigned int BuildRRCTransmission(std::vector<std::complex<float>> &tx_buffer)

{
  unsigned int samplesInBuffer = 0;
  unsigned int samps_to_transmit = 10000;
  std::complex<float> complex_symbol;
  unsigned int h_len = 2 * RRC_SAMPS_PER_SYM * RRC_FILTER_SEMILENGTH + 1;

  for (unsigned int j = 0; j < samps_to_transmit; j++) {
    // samplesAddedToBuffer++;
    samplesInBuffer++;
    // generate a random QPSK symbol until within a filter length of the end
    if ((j % RRC_SAMPS_PER_SYM == 0) &&
        (samplesInBuffer < tx_buffer.size() - 2 * h_len)) {
      complex_symbol.real(0.5 * (float)roundf((float)rand() / (float)RAND_MAX) -
                          0.25);
      complex_symbol.imag(0.5 * (float)roundf((float)rand() / (float)RAND_MAX) -
                          0.25);
    }
    // zero insert to interpolate
    else {
      complex_symbol.real(0.0);
      complex_symbol.imag(0.0);
    }

    firfilt_crcf_push(rrc_filt, complex_symbol);
    firfilt_crcf_execute(rrc_filt, &tx_buffer[j]);
  }
  return samps_to_transmit;
}

// ========================================================================
//  FUNCTION:  Build OFDM Transmission
// ========================================================================

int BuildOFDMTransmission(std::vector<std::complex<float>> &tx_buffer,
                          Interferer Int, struct node_parameters *np)

{
  unsigned int num_subcarriers = 2 * (unsigned int)(np->tx_rate / 30e3);
  unsigned int cp_len = OFDM_CP_LENGTH;

  // header and payload for frame generators
  unsigned char header[OFDM_HEADER_LENGTH];
  unsigned char payload[OFDM_PAYLOAD_LENGTH];

  ofdmflexframegen_assemble(ofdm_fg, header, payload, OFDM_PAYLOAD_LENGTH);

  int samps_to_transmit = 0;
  int frame_complete = 0;
  int j = 0;
  while (!frame_complete &&
         (unsigned int)samps_to_transmit <
             TX_BUFFER_LENGTH - num_subcarriers - OFDM_CP_LENGTH) {
    frame_complete = ofdmflexframegen_writesymbol(
        ofdm_fg, &tx_buffer[j * (num_subcarriers + cp_len)]);
    samps_to_transmit += num_subcarriers + cp_len;
    j++;
  }

  // calculate soft gain
  float g = powf(10.0f, Int.tx_gain_soft / 20.0f);

  // reduce amplitude of signal to avoid clipping
  float max_real = 0.0f;
  for (int j = 0; j < samps_to_transmit; j++) {
    tx_buffer[j] *= g;
    if (tx_buffer[j].real() > max_real)
      max_real = tx_buffer[j].real();
  }
  return samps_to_transmit;
}

// ========================================================================
//  FUNCTION:  Log transmission parameters
// ========================================================================
void log_tx_parameters() {

  // update current time
  struct timeval tv;
  gettimeofday(&tv, NULL);

  // create string of actual file location
  char file_name[50];
  strcpy(file_name, "./logs/");
  strcat(file_name, phy_tx_log_file);

  // open file, append parameters, and close
  std::ofstream log_file;
  log_file.open(file_name, std::ofstream::out | std::ofstream::binary |
                               std::ofstream::app);
  if (log_file.is_open()) {
    log_file.write((char *)&tv, sizeof(tv));
    log_file.write((char *)&currentTxFreq, sizeof(currentTxFreq));
  } else {
    std::cerr << "Error opening log file:" << file_name << std::endl;
  }
  log_file.close();
}

// ========================================================================
//  FUNCTION:  Transmit Interference
// ========================================================================

void TransmitInterference(Interferer Int,
                          std::vector<std::complex<float>> &tx_buffer,
                          int samplesInBuffer, node_parameters np,
                          scenario_parameters sp) {
  int tx_samp_count = 0;
  int usrp_samps = USRP_BUFFER_LENGTH;

  // if(log_phy_tx_flag)
  //		log_tx_parameters();

  Int.metadata_tx.start_of_burst = false;
  Int.metadata_tx.end_of_burst = false;
  Int.metadata_tx.has_time_spec = false;

  Int.usrp_tx->get_device()->send(&tx_buffer[0], 0, Int.metadata_tx,
                                  uhd::io_type_t::COMPLEX_FLOAT32,
                                  uhd::device::SEND_MODE_ONE_PACKET);

  Int.metadata_tx.start_of_burst = true;

  while (tx_samp_count < samplesInBuffer) {
    // modifications for the last group of samples generated
    if (samplesInBuffer - tx_samp_count <= USRP_BUFFER_LENGTH) {
      usrp_samps = samplesInBuffer - tx_samp_count;
      Int.metadata_tx.end_of_burst = true;
    }
    Int.usrp_tx->get_device()->send(
        &tx_buffer[tx_samp_count], usrp_samps, Int.metadata_tx,
        uhd::io_type_t::COMPLEX_FLOAT32, uhd::device::SEND_MODE_ONE_PACKET);

    Int.metadata_tx.start_of_burst = false;

    // update number of tx samples remaining
    tx_samp_count += USRP_BUFFER_LENGTH;
  } // usrp transmit while loop
}

// ========================================================================
//  FUNCTION:  Set Tx Freq for Frequency Hopping
// ========================================================================
void ChangeFrequency(Interferer Int) {

  switch (Int.tx_freq_hop_type) {
  case (ALTERNATING):
    if (currentTxFreq == Int.tx_freq_hop_max) {
      currentTxFreq = Int.tx_freq_hop_min;
    } else {
      currentTxFreq = Int.tx_freq_hop_max;
    }
    break;
  case (SWEEP):
    currentTxFreq += (freqIncrement * freqCoeff);
    if ((currentTxFreq > Int.tx_freq_hop_max) ||
        (currentTxFreq < Int.tx_freq_hop_min)) {
      freqCoeff = freqCoeff * -1;
      currentTxFreq = currentTxFreq + (2 * freqIncrement * freqCoeff);
    }
    break;
  case (RANDOM):
    currentTxFreq = rand() % freqWidth + Int.tx_freq_hop_min;
    break;
  }
  Int.usrp_tx->set_tx_freq(currentTxFreq);
  printf("Set transmit frequency to %f\n", currentTxFreq);
}

// ========================================================================
//  FUNCTION:  Perform Duty Cycle ON
// ========================================================================
void PerformDutyCycle_On(Interferer Int, node_parameters np,
                         scenario_parameters sp, float time_onCycle) {
  std::vector<std::complex<float>> tx_buffer(2 * TX_BUFFER_LENGTH);
  unsigned int samplesInBuffer = 0;
  unsigned int randomFlag = (Int.interference_type == (NOISE)) ? 1 : 0;
  timer_tic(onTimer);

  printf("Interferer turning on\n");

  while (timer_toc(onTimer) < time_onCycle) {

    // determine if we need to freq hop
    float a = timer_toc(dwellTimer);
    if ((Int.tx_freq_hop_type != (NONE)) && (a >= Int.tx_freq_hop_dwell_time)) {
      timer_tic(dwellTimer);
      ChangeFrequency(Int);
      // usleep(100);
    }

    // Generate One Frame of Data to Transmit
    switch (Int.interference_type) {
    case (CW):
    case (NOISE):
      FillBufferForTransmission(randomFlag, tx_buffer);
      samplesInBuffer = tx_buffer.size();
      break;

    case (GMSK):
      samplesInBuffer = BuildGMSKTransmission(tx_buffer, Int);
      break;

    case (RRC):
      samplesInBuffer = BuildRRCTransmission(tx_buffer);
      break;

    case (OFDM):
      samplesInBuffer = BuildOFDMTransmission(tx_buffer, Int, &np);
      break;

    } // interference type switch

    /*Receive_command_from_controller(&Int, &np, &sp);
    if (sig_terminate)
                    break;*/
    TransmitInterference(Int, tx_buffer, samplesInBuffer, np, sp);
    Receive_command_from_controller(&Int, &np, &sp);
    if (sig_terminate)
      break;
  }
}

// ========================================================================
//  FUNCTION:  Perform Duty Cycle OFF
// ========================================================================

void PerformDutyCycle_Off(Interferer Int, node_parameters np,
                          scenario_parameters sp, float time_offCycle) {
  printf("Interferer turning off\n");

  timer_tic(onTimer);
  while (timer_toc(onTimer) < time_offCycle) {
    usleep(100);
    Receive_command_from_controller(&Int, &np, &sp);
    if (sig_terminate) {
      break;
    }
  }
}

// ==========================================================================
// ==========================================================================
// ==========================================================================
//  MAIN PROGRAM
// ==========================================================================
// ==========================================================================
// ==========================================================================

int main(int argc, char **argv) {
  // register signal handlers
  signal(SIGINT, terminate);
  signal(SIGQUIT, terminate);
  signal(SIGTERM, terminate);

  // set default values
  time_t run_time = DEFAULT_RUN_TIME;
  char *controller_ipaddr = (char *)DEFAULT_CONTROLLER_IP_ADDRESS;
  TCP_controller = socket(AF_INET, SOCK_STREAM, 0);

  // validate TCP Controller
  if (TCP_controller < 0) {
    printf("ERROR: Receiver Failed to Create Client Socket\n");
    exit(EXIT_FAILURE);
  }

  // get command line parameters
  int d;
  while ((d = getopt(argc, argv, "ht:a:")) != EOF) {
    switch (d) {
    case 'h':
      help_CRTS_interferer();
      return 0;
    case 't':
      run_time = atof(optarg);
      break;
    case 'a':
      controller_ipaddr = optarg;
      break;
    }
  }

  // Parameters for connecting to server
  struct sockaddr_in controller_addr;
  memset(&controller_addr, 0, sizeof(controller_addr));
  controller_addr.sin_family = AF_INET;
  controller_addr.sin_addr.s_addr = inet_addr(controller_ipaddr);
  controller_addr.sin_port = htons(4444);

  // Attempt to connect client socket to server
  int connect_status =
      connect(TCP_controller, (struct sockaddr *)&controller_addr,
              sizeof(controller_addr));
  if (connect_status) {
    printf("Failed to Connect to server.\n");
    exit(EXIT_FAILURE);
  }

  uhd::msg::register_handler(&uhd_quiet);

  // Create node parameters struct and interferer object
  struct node_parameters np;
  struct scenario_parameters sp;
  Interferer Int;

  // Set metadata for interferer
  // Int.metadata_tx.start_of_burst = false;
  // Int.metadata_tx.end_of_burst = false;
  // Int.metadata_tx.has_time_spec = false;

  // Read initial scenario info from controller
  printf("Receiving command from controller\n");
  Receive_command_from_controller(&Int, &np, &sp);
  // fcntl(TCP_controller, F_SETFL, O_NONBLOCK);

  //===================================================================
  // Set up GMSK objects
  //===================================================================

  // gmsk frame generator
  dprintf("Creating GMSK frame generator\n");
  gmsk_fg = gmskframegen_create();

  // usrp_tx_rate = Int.usrp_tx->get_tx_rate();

  // double tx_resamp_rate = usrp_tx_rate / tx_rate;
  // dprintf("resample rate for arbitrary resampler: %f \n", tx_resamp_rate);

  // half-band interpolator
  dprintf("Creating interpolator\n");
  interp = resamp2_crcf_create(7, 0.0f, 40.0f);

  // add arbitrary resampling component
  // resamp_crcf resamp = resamp_crcf_create(tx_resamp_rate,7,0.4f,30.0f,64);
  // resamp_crcf_setrate(resamp, tx_resamp_rate);

  // ================================================================
  // Set up RRC transmit
  // ================================================================
  unsigned int h_len = 2 * RRC_SAMPS_PER_SYM * RRC_FILTER_SEMILENGTH + 1;
  float h[h_len];
  liquid_firdes_rrcos(RRC_SAMPS_PER_SYM, RRC_FILTER_SEMILENGTH, RRC_BETA, 0.0,
                      h);
  rrc_filt = firfilt_crcf_create(h, h_len);

  // ================================================================
  // Set up OFDM transmit
  // ================================================================
  unsigned int num_subcarriers = 2 * (unsigned int)(np.tx_rate / 30e3);
  unsigned char *subcarrierAlloc = NULL;
  ofdmflexframegenprops_init_default(&fgprops);
  ofdm_fg =
      ofdmflexframegen_create(num_subcarriers, OFDM_CP_LENGTH,
                              OFDM_TAPER_LENGTH, subcarrierAlloc, &fgprops);

  // ================================================================
  // BEGIN: Main Service Loop
  // ================================================================
  srand(431);
  sig_terminate = 0;
  float time_onCycle = (Int.period * Int.duty_cycle);
  float time_offCycle = (Int.period * (1 - Int.duty_cycle));

  // wait for start time and calculate stop time
  struct timeval tv;
  time_t time_s;
  stop_time_s = sp.start_time_s + run_time;
  while (1) {
    Receive_command_from_controller(&Int, &np, &sp);
    gettimeofday(&tv, NULL);
    time_s = tv.tv_sec;
    if (time_s >= sp.start_time_s)
      break;
  }

  onTimer = timer_create();
  dwellTimer = timer_create();
  timer_tic(dwellTimer);

  while (time_s < stop_time_s) {
    Receive_command_from_controller(&Int, &np, &sp);

    if (sig_terminate)
      break;

    PerformDutyCycle_On(Int, np, sp, time_onCycle);
    PerformDutyCycle_Off(Int, np, sp, time_offCycle);

    if (sig_terminate)
      break;

    // update current time
    gettimeofday(&tv, NULL);
    time_s = tv.tv_sec;

  } // end main "for" interation loop
  // ================================================================
  // END: Main Service Loop
  // ================================================================

  printf("Sending termination message to controller\n");
  char term_message = terminate_msg;
  write(TCP_controller, &term_message, 1);
}
