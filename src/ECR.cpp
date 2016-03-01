#include <stdio.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <math.h>
#include <complex>
#include <liquid/liquid.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <sys/time.h>
#include <sched.h>
#include <bitset>
#include "CRTS.hpp"
#include "ECR.hpp"
#include "TUN.hpp"

// EDIT INCLUDE START FLAG
#include "../cognitive_engines/CE_Template.hpp"
#include "../cognitive_engines/CE_Subcarrier_Alloc.hpp"
#include "../cognitive_engines/CE_Mod_Adaptation.hpp"
#include "../cognitive_engines/CE_Two_Channel_DSA_Spectrum_Sensing.hpp"
#include "../cognitive_engines/CE_Two_Channel_DSA_PU.hpp"
#include "../cognitive_engines/CE_FEC_Adaptation.hpp"
#include "../cognitive_engines/CE_Two_Channel_DSA_Link_Reliability.hpp"
#include "../cognitive_engines/CE_Control_and_Feedback_Test.hpp"
#include "../cognitive_engines/CE_Simultaneous_RX_And_Sensing.hpp"
#include "../cognitive_engines/CE_Throughput_Test.hpp"
// EDIT INCLUDE END FLAG

#define DEBUG 0
#if DEBUG == 1
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

void uhd_msg_handler(uhd::msg::type_t type, const std::string &msg);
int ExtensibleCognitiveRadio::uhd_msg;

// Constructor
ExtensibleCognitiveRadio::ExtensibleCognitiveRadio() {

  // Set initial timeout value for executing CE
  ce_timeout_ms = 1000;

  // set internal properties
  tx_params.numSubcarriers = 32;
  tx_params.cp_len = 16;
  tx_params.taper_len = 4;
  tx_params.subcarrierAlloc = (unsigned char *)malloc(32 * sizeof(char));
  tx_params.payload_sym_length = 288 * 8; // 256 * 8;
  rx_params.numSubcarriers = 32;
  rx_params.cp_len = 16;
  rx_params.taper_len = 4;
  rx_params.subcarrierAlloc = (unsigned char *)malloc(32 * sizeof(char));

  // use liquid default subcarrier allocation
  ofdmframe_init_default_sctype(32, tx_params.subcarrierAlloc);
  ofdmframe_init_default_sctype(32, rx_params.subcarrierAlloc);

  // determine how many subcarriers carry data
  numDataSubcarriers = 0;
  for (unsigned int i = 0; i < tx_params.numSubcarriers; i++) {
    if (tx_params.subcarrierAlloc[i] == OFDMFRAME_SCTYPE_DATA)
      numDataSubcarriers++;
  }
  update_tx_data_rate = 1;

  CE_metrics.payload = NULL;

  // Initialize header to all zeros
  memset(tx_header, 0, sizeof(tx_header));

  // store known payload for BER calculations
  msequence ms = msequence_create_default(CRTS_CR_PACKET_SR_LEN);
  // first several bytes are rewritten since they are used for the packet number
  for (int i=0; i<CRTS_CR_PACKET_NUM_LEN; i++)
    known_net_payload[i] = msequence_generate_symbol(ms,8);
  for (int i=0; i<CRTS_CR_PACKET_LEN-CRTS_CR_PACKET_NUM_LEN; i++){
    known_net_payload[i] = msequence_generate_symbol(ms,8);
  }

  // enable physical layer events for ce
  ce_phy_events = true;

  // initialize frame generator setting
  ofdmflexframegenprops_init_default(&tx_params.fgprops);
  tx_params.fgprops.check = LIQUID_CRC_32;
  tx_params.fgprops.fec0 = LIQUID_FEC_HAMMING128;
  tx_params.fgprops.fec1 = LIQUID_FEC_NONE;
  tx_params.fgprops.mod_scheme = LIQUID_MODEM_QAM4;

  // copy tx_params to tx_params_updated and initialize update flags to 0
  tx_params_updated = tx_params;
  update_tx_flag = 0;
  update_usrp_tx = 0;
  recreate_fg = 0;

  // create frame generator
  fg = ofdmflexframegen_create(tx_params.numSubcarriers, tx_params.cp_len,
                               tx_params.taper_len, tx_params.subcarrierAlloc,
                               &tx_params.fgprops);

  // allocate memory for frame generator output (single OFDM symbol)
  fgbuffer_len = tx_params.numSubcarriers + tx_params.cp_len;
  fgbuffer =
      (std::complex<float> *)malloc(fgbuffer_len * sizeof(std::complex<float>));

  // create frame synchronizer
  fs = ofdmflexframesync_create(rx_params.numSubcarriers, rx_params.cp_len,
                                rx_params.taper_len, rx_params.subcarrierAlloc,
                                rxCallback, (void *)this);

  // initialize update flags to 0
  update_rx_flag = 0;
  update_usrp_rx = 0;
  recreate_fs = 0;
  rx_stat_tracking = 0;

  // register UHD message handler
  uhd::msg::register_handler(&uhd_msg_handler);

  // create usrp objects
  uhd::device_addr_t dev_addr;
  usrp_tx = uhd::usrp::multi_usrp::make(dev_addr);
  usrp_rx = uhd::usrp::multi_usrp::make(dev_addr);
  usrp_rx->set_rx_antenna("RX2", 0);
  usrp_tx->set_tx_antenna("TX/RX", 0);

  // Create TUN interface
  dprintf("Creating tun interface\n");
  strcpy(tun_name, "tunCRTS");
  sprintf(systemCMD, "sudo ip tuntap add dev %s mode tun", tun_name);
  system(systemCMD);
  dprintf("Bringing up tun interface\n");
  dprintf("Connecting to tun interface\n");
  sprintf(systemCMD, "sudo ip link set dev %s up", tun_name);
  system(systemCMD);

  // Get reference to TUN interface
  tunfd = tun_alloc(tun_name, IFF_TUN);

  // start the ce without sensing by default
  ce_sensing_flag = 0;

  // create and start rx thread
  dprintf("Starting rx thread...\n");
  rx_state = RX_CONTINUOUS;            // receiver is not running initially
  rx_thread_running = true;            // receiver thread IS running initially
  pthread_mutex_init(&rx_mutex, NULL); // receiver mutex
  pthread_cond_init(&rx_cond, NULL);   // receiver condition
  pthread_create(&rx_process, NULL, ECR_rx_worker, (void *)this);

  // create and start tx thread
  frame_num = 0;
  tx_state = TX_STOPPED;    // transmitter is not running initially
  tx_thread_running = true; // transmitter thread IS running initially
  pthread_mutex_init(&tx_mutex, NULL); // transmitter mutex
  pthread_cond_init(&tx_cond, NULL);   // transmitter condition
  pthread_create(&tx_process, NULL, ECR_tx_worker, (void *)this);

  // Start CE thread
  dprintf("Starting CE thread...\n");
  ce_running = false;       // ce is not running initially
  ce_thread_running = true; // ce thread IS running initially
  pthread_mutex_init(&CE_mutex, NULL);
  pthread_mutex_init(&CE_fftw_mutex, NULL);
  pthread_cond_init(&CE_execute_sig, NULL);
  pthread_cond_init(&CE_cond, NULL); // cognitive engine condition
  pthread_create(&CE_process, NULL, ECR_ce_worker, (void *)this);

  // Set thread priorities
  /*int rval = 0;
      struct sched_param param;
      int policy;
      param.sched_priority = 99;
      rval = pthread_setschedparam(rx_process, SCHED_FIFO, &param);
      rval = pthread_getschedparam(rx_process, &policy, &param);
      printf("Rx thread priority: %d\n", param.sched_priority);
      */

  /*param.sched_priority = 0;
      rval = pthread_setschedparam(tx_process, SCHED_FIFO, &param);
      rval = pthread_getschedparam(tx_process, &policy, &param);
      printf("Tx thread priority: %d\n", param.sched_priority);
      */

  /*param.sched_priority = 0;
      rval = pthread_setschedparam(CE_process, SCHED_FIFO, &param);
      rval = pthread_getschedparam(CE_process, &policy, &param);
      printf("CE thread priority: %d\n", param.sched_priority);

  rval = pthread_setschedparam(0, SCHED_FIFO, &param);
      rval = pthread_getschedparam(0, &policy, &param);
      printf("Main thread priority: %d\n", param.sched_priority);
      */

  // Constrain threads to specific cores
  /*cpu_set_t set;

  CPU_ZERO(&set);
  CPU_SET(0, &set);
  CPU_SET(1, &set);
  sched_setaffinity(rx_process, sizeof(cpu_set_t), &set);

  CPU_ZERO(&set);
  CPU_SET(2, &set);
  sched_setaffinity(tx_process, sizeof(cpu_set_t), &set);

  CPU_ZERO(&set);
  CPU_SET(2, &set);
  sched_setaffinity(CE_process, sizeof(cpu_set_t), &set);

CPU_ZERO(&set);
  CPU_SET(2, &set);
  sched_setaffinity(0, sizeof(cpu_set_t), &set);
  */

  // initialize default tx values
  dprintf("Initializing USRP settings...\n");
  set_tx_freq(460.0e6f);
  set_tx_rate(500e3);
  set_tx_gain_soft(-12.0f);
  set_tx_gain_uhd(0.0f);

  // initialize default rx values
  set_rx_freq(460.0e6f);
  set_rx_rate(500e3);
  set_rx_gain_uhd(0.0f);

  // reset transceiver
  reset_tx();
  reset_rx();

  dprintf("Finished creating ECR\n");
}

// Destructor
ExtensibleCognitiveRadio::~ExtensibleCognitiveRadio() {

  // close log files
  log_tx_fstream.close();
  log_rx_fstream.close();

  // if (ce_running)
  stop_ce();

  // signal condition (tell ce worker to continue)
  dprintf("destructor signaling ce condition...\n");
  ce_thread_running = false;
  pthread_cond_signal(&CE_cond);

  dprintf("destructor joining ce thread...\n");
  void *ce_exit_status;
  pthread_join(CE_process, &ce_exit_status);

  dprintf("Stopping transceiver\n");
  stop_rx();
  stop_tx();

  // sleep so tx/rx threads are ready for signal
  usleep(1e4);

  // signal condition (tell rx worker to continue)
  dprintf("destructor signaling rx condition...\n");
  rx_thread_running = false;
  pthread_cond_signal(&rx_cond);

  dprintf("destructor joining rx thread...\n");
  void *rx_exit_status;
  pthread_join(rx_process, &rx_exit_status);

  // signal condition (tell tx worker to continue)
  dprintf("destructor signaling tx condition...\n");
  tx_thread_running = false;
  pthread_cond_signal(&tx_cond);

  dprintf("destructor joining tx thread...\n");
  void *tx_exit_status;
  pthread_join(tx_process, &tx_exit_status);

  // destroy ce threading objects
  dprintf("destructor destroying ce mutex...\n");
  pthread_mutex_destroy(&CE_mutex);
  dprintf("destructor destroying ce condition...\n");
  pthread_cond_destroy(&CE_cond);

  // destroy rx threading objects
  dprintf("destructor destroying rx mutex...\n");
  pthread_mutex_destroy(&rx_mutex);
  dprintf("destructor destroying rx condition...\n");
  pthread_cond_destroy(&rx_cond);

  // destroy tx threading objects
  dprintf("destructor destroying tx mutex...\n");
  pthread_mutex_destroy(&tx_mutex);
  dprintf("destructor destroying tx condition...\n");
  pthread_cond_destroy(&tx_cond);

  // destroy framing objects
  dprintf("destructor destroying other objects...\n");
  ofdmflexframegen_destroy(fg);
  ofdmflexframesync_destroy(fs);

  // free memory for subcarrier allocation if necessary
  if (tx_params.subcarrierAlloc)
    free(tx_params.subcarrierAlloc);

  // close the TUN interface file descriptor
  dprintf("destructor closing the TUN interface file descriptor\n");
  close(tunfd);

  dprintf("destructor bringing down TUN interface\n");
  sprintf(systemCMD, "sudo ip link set dev %s down", tun_name);
  system(systemCMD);

  dprintf("destructor deleting TUN interface\n");
  sprintf(systemCMD, "sudo ip tuntap del dev %s mode tun", tun_name);
  system(systemCMD);
}

///////////////////////////////////////////////////////////////////////
// Cognitive engine methods
///////////////////////////////////////////////////////////////////////

void ExtensibleCognitiveRadio::set_ce(char *ce, int argc, char **argv) {
  ///@cond INTERNAL
  // EDIT SET CE START FLAG
  if(!strcmp(ce, "CE_Template"))
    CE = new CE_Template(argc, argv);
  if(!strcmp(ce, "CE_Subcarrier_Alloc"))
    CE = new CE_Subcarrier_Alloc(argc, argv);
  if(!strcmp(ce, "CE_Mod_Adaptation"))
    CE = new CE_Mod_Adaptation(argc, argv);
  if(!strcmp(ce, "CE_Two_Channel_DSA_Spectrum_Sensing"))
    CE = new CE_Two_Channel_DSA_Spectrum_Sensing(argc, argv);
  if(!strcmp(ce, "CE_Two_Channel_DSA_PU"))
    CE = new CE_Two_Channel_DSA_PU(argc, argv);
  if(!strcmp(ce, "CE_FEC_Adaptation"))
    CE = new CE_FEC_Adaptation(argc, argv);
  if(!strcmp(ce, "CE_Two_Channel_DSA_Link_Reliability"))
    CE = new CE_Two_Channel_DSA_Link_Reliability(argc, argv);
  if(!strcmp(ce, "CE_Control_and_Feedback_Test"))
    CE = new CE_Control_and_Feedback_Test(argc, argv);
  if(!strcmp(ce, "CE_Simultaneous_RX_And_Sensing"))
    CE = new CE_Simultaneous_RX_And_Sensing(argc, argv);
  if(!strcmp(ce, "CE_Throughput_Test"))
    CE = new CE_Throughput_Test(argc, argv);
  // EDIT SET CE END FLAG
  ///@endcond
}

void ExtensibleCognitiveRadio::start_ce() {
  // set ce running flag
  ce_running = true;
  // signal condition for the ce to start listening for events of interest
  pthread_cond_signal(&CE_cond);
}

void ExtensibleCognitiveRadio::stop_ce() {
  // reset ce running flag
  ce_running = false;
}

void ExtensibleCognitiveRadio::set_ce_timeout_ms(double new_timeout_ms) {
  ce_timeout_ms = new_timeout_ms;
}

double ExtensibleCognitiveRadio::get_ce_timeout_ms() { return ce_timeout_ms; }

void ExtensibleCognitiveRadio::set_ce_sensing(int ce_sensing) {
  ce_sensing_flag = ce_sensing;
}

void uhd_msg_handler(uhd::msg::type_t type, const std::string &msg) {

  if ((!strcmp(msg.c_str(), "O")) || (!strcmp(msg.c_str(), "D")))
    ExtensibleCognitiveRadio::uhd_msg = 1;
  if (!strcmp(msg.c_str(), "U"))
    ExtensibleCognitiveRadio::uhd_msg = 2;
}

///////////////////////////////////////////////////////////////////////
// Network methods
///////////////////////////////////////////////////////////////////////

// set the ip address for the virtual network interface
void ExtensibleCognitiveRadio::set_ip(char *ip) {
  sprintf(systemCMD, "sudo ifconfig %s %s netmask 255.255.255.0", tun_name, ip);
  system(systemCMD);
}

// set the length of the tun interface queue tx length
void ExtensibleCognitiveRadio::set_tx_queue_len(int queue_len) {
  sprintf(systemCMD, "sudo ifconfig %s txqueuelen %i", tun_name, queue_len);
  system(systemCMD);
}

////////////////////////////////////////////////////////////////////////
// Transmit methods
////////////////////////////////////////////////////////////////////////

// start transmitter
void ExtensibleCognitiveRadio::start_tx() {
  // set tx running flag
  tx_state = TX_CONTINUOUS;
  // signal condition (tell tx worker to start)
  pthread_cond_signal(&tx_cond);
}

// start transmitter
void ExtensibleCognitiveRadio::start_tx_for_frames(int _num_tx_frames) {
  // set tx running flag and number of tx frames
  tx_state = TX_FOR_FRAMES;
  num_tx_frames = _num_tx_frames;
  // signal condition (tell tx worker to start)
  pthread_cond_signal(&tx_cond);
}

// stop transmitter
void ExtensibleCognitiveRadio::stop_tx() {
  // reset tx running flag
  tx_state = TX_STOPPED;
}

// reset transmitter objects and buffers
void ExtensibleCognitiveRadio::reset_tx() { ofdmflexframegen_reset(fg); }

// set transmitter frequency
void ExtensibleCognitiveRadio::set_tx_freq(double _tx_freq) {
  // pthread_mutex_lock(&tx_mutex);
  tx_params_updated.tx_freq = _tx_freq;
  tx_params_updated.tx_dsp_freq = 0.0;
  update_tx_flag = 1;
  update_usrp_tx = 1;
  // usrp_tx->set_tx_freq(_tx_freq);
  // pthread_mutex_unlock(&tx_mutex);
}

// set transmitter frequency
void ExtensibleCognitiveRadio::set_tx_freq(double _tx_freq, double _dsp_freq) {
  // pthread_mutex_lock(&tx_mutex);
  tx_params_updated.tx_freq = _tx_freq;
  tx_params_updated.tx_dsp_freq = _dsp_freq;
  update_tx_flag = 1;
  update_usrp_tx = 1;

  // uhd::tune_request_t tune;
  // tune.rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
  // tune.dsp_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
  // tune.rf_freq = _tx_freq;
  // tune.dsp_freq = _dsp_freq;

  // usrp_tx->set_tx_freq(tune);
  // pthread_mutex_unlock(&tx_mutex);
}

// get transmitter state
int ExtensibleCognitiveRadio::get_tx_state() {
  return tx_state;
}

// get transmitter frequency
double ExtensibleCognitiveRadio::get_tx_freq() {
  return tx_params.tx_freq + tx_params.tx_dsp_freq;
}

// set transmitter sample rate
void ExtensibleCognitiveRadio::set_tx_rate(double _tx_rate) {
  tx_params_updated.tx_rate = _tx_rate;
  update_tx_flag = 1;
  update_usrp_tx = 1;
}

// get transmitter sample rate
double ExtensibleCognitiveRadio::get_tx_rate() { return tx_params.tx_rate; }

// set transmitter software gain
void ExtensibleCognitiveRadio::set_tx_gain_soft(double _tx_gain_soft) {
  tx_params_updated.tx_gain_soft = _tx_gain_soft;
  update_tx_flag = 1;
}

// get transmitter software gain
double ExtensibleCognitiveRadio::get_tx_gain_soft() {
  return tx_params_updated.tx_gain_soft;
}

// set transmitter hardware (UHD) gain
void ExtensibleCognitiveRadio::set_tx_gain_uhd(double _tx_gain_uhd) {
  tx_params_updated.tx_gain_uhd = _tx_gain_uhd;
  update_tx_flag = 1;
  update_usrp_tx = 1;
}

// get transmitter hardware (UHD) gain
double ExtensibleCognitiveRadio::get_tx_gain_uhd() {
  return tx_params.tx_gain_uhd;
}

// set modulation scheme
void ExtensibleCognitiveRadio::set_tx_modulation(int mod_scheme) {
  tx_params_updated.fgprops.mod_scheme = mod_scheme;
  update_tx_flag = 1;
}

// get modulation scheme
int ExtensibleCognitiveRadio::get_tx_modulation() {
  return tx_params.fgprops.mod_scheme;
}

// decrease modulation order
void ExtensibleCognitiveRadio::decrease_tx_mod_order() {
  unsigned int mod = tx_params.fgprops.mod_scheme;

  // Check to see if modulation order is already minimized
  if (mod != 1 && mod != 9 && mod != 17 && mod != 25 && mod != 32) {
    tx_params_updated.fgprops.mod_scheme--;
    update_tx_flag = 1;
  }
}

// increase modulation order
void ExtensibleCognitiveRadio::increase_tx_mod_order() {
  unsigned int mod = tx_params.fgprops.mod_scheme;

  // check to see if modulation order is already maximized
  if (mod != 8 && mod != 16 && mod != 24 && mod != 31 && mod != 38) {
    tx_params_updated.fgprops.mod_scheme++;
    update_tx_flag = 1;
  }
}

// set CRC scheme
void ExtensibleCognitiveRadio::set_tx_crc(int crc_scheme) {
  tx_params_updated.fgprops.check = crc_scheme;
  update_tx_flag = 1;
}

// get CRC scheme
int ExtensibleCognitiveRadio::get_tx_crc() { return tx_params.fgprops.check; }

// set FEC0
void ExtensibleCognitiveRadio::set_tx_fec0(int fec_scheme) {
  tx_params_updated.fgprops.fec0 = fec_scheme;
  update_tx_flag = 1;
}

// get FEC0
int ExtensibleCognitiveRadio::get_tx_fec0() { return tx_params.fgprops.fec0; }

// set FEC1
void ExtensibleCognitiveRadio::set_tx_fec1(int fec_scheme) {
  tx_params_updated.fgprops.fec1 = fec_scheme;
  update_tx_flag = 1;
}

// get FEC1
int ExtensibleCognitiveRadio::get_tx_fec1() { return tx_params.fgprops.fec1; }

// set number of subcarriers
void ExtensibleCognitiveRadio::set_tx_subcarriers(
    unsigned int _numSubcarriers) {
  tx_params_updated.numSubcarriers = _numSubcarriers;
  update_tx_flag = 1;
  recreate_fg = 1;
}

// get number of subcarriers
unsigned int ExtensibleCognitiveRadio::get_tx_subcarriers() {
  return tx_params.numSubcarriers;
}
void ExtensibleCognitiveRadio::set_tx_subcarrier_alloc(char *_subcarrierAlloc) {

  tx_params_updated.subcarrierAlloc =
      (unsigned char *)realloc((void *)tx_params_updated.subcarrierAlloc,
                               tx_params_updated.numSubcarriers);

  if (_subcarrierAlloc) {
    memcpy(tx_params_updated.subcarrierAlloc, _subcarrierAlloc,
           tx_params_updated.numSubcarriers);
  } else {
    ofdmframe_init_default_sctype(tx_params_updated.numSubcarriers,
                                  tx_params_updated.subcarrierAlloc);
  }

  // calculate the number of data subcarriers
  numDataSubcarriers = 0;
  for (unsigned int i = 0; i < tx_params_updated.numSubcarriers; i++) {
    if (tx_params_updated.subcarrierAlloc[i] == OFDMFRAME_SCTYPE_DATA)
      numDataSubcarriers++;
  }

  update_tx_flag = 1;
  recreate_fg = 1;
}

// get subcarrier allocation
void ExtensibleCognitiveRadio::get_tx_subcarrier_alloc(char *subcarrierAlloc) {
  memcpy(subcarrierAlloc, tx_params.subcarrierAlloc, tx_params.numSubcarriers);
}

// set cp_len
void ExtensibleCognitiveRadio::set_tx_cp_len(unsigned int _cp_len) {
  tx_params_updated.cp_len = _cp_len;
  update_tx_flag = 1;
  recreate_fg = 1;
}

// get cp_len
unsigned int ExtensibleCognitiveRadio::get_tx_cp_len() {
  return tx_params.cp_len;
}

// set taper_len
void ExtensibleCognitiveRadio::set_tx_taper_len(unsigned int _taper_len) {
  tx_params_updated.taper_len = _taper_len;
  update_tx_flag = 1;
  recreate_fg = 1;
}

// get taper_len
unsigned int ExtensibleCognitiveRadio::get_tx_taper_len() {
  return tx_params.taper_len;
}

// set control info (must have length 6)
void ExtensibleCognitiveRadio::set_tx_control_info(
    unsigned char *_control_info) {
  memcpy(&tx_header[2], _control_info, 6 * sizeof(unsigned char));
}

// get control info
void ExtensibleCognitiveRadio::get_tx_control_info(
    unsigned char *_control_info) {
  memcpy(_control_info, &tx_header[2], 6 * sizeof(unsigned char));
}

// set tx payload length
void ExtensibleCognitiveRadio::set_tx_payload_sym_len(unsigned int len) {
  tx_params_updated.payload_sym_length = len;
  update_tx_flag = 1;
}

// get approximate physical layer data rate
double ExtensibleCognitiveRadio::get_tx_data_rate() {

  if (update_tx_data_rate) {
    double fec_rate = fec_get_rate((fec_scheme)tx_params.fgprops.fec0) *
                      fec_get_rate((fec_scheme)tx_params.fgprops.fec1);
    double crc_len =
        (double)(crc_get_length((crc_scheme)tx_params.fgprops.check) * 8);
    double bps = (double)modulation_types[tx_params.fgprops.mod_scheme].bps;
    double syms = (double)((tx_params.payload_sym_length * bps + crc_len) /
                           (fec_rate * bps));
    int ofdm_syms = 3 + (int)ceilf(224.0 / numDataSubcarriers) +
                    (int)ceilf(syms / numDataSubcarriers);
    tx_data_rate =
        (double)tx_params.payload_sym_length * bps * tx_params.tx_rate /
        (double)((tx_params_updated.cp_len + tx_params_updated.numSubcarriers) *
                 ofdm_syms);
    update_tx_data_rate = 0;
    printf("payload syms:%i\n", tx_params.payload_sym_length);
    printf("bps: %f\n", bps);
    printf("tx rate: %f\n", tx_params.tx_rate);
    printf("\nUpdated tx rate: %f\n\n", tx_data_rate);
  }

  return tx_data_rate;
}

// get control info
void ExtensibleCognitiveRadio::get_rx_control_info(
    unsigned char *_control_info) {
  memcpy(_control_info, CE_metrics.control_info, 6 * sizeof(unsigned char));
}

// update the actual parameters being used by the transmitter
void ExtensibleCognitiveRadio::update_tx_params() {

  // copy all the new parameters
  tx_params = tx_params_updated;

  // update the USRP
  if (update_usrp_tx) {
    usrp_tx->set_tx_gain(tx_params.tx_gain_uhd);
    usrp_tx->set_tx_rate(tx_params.tx_rate);

    uhd::tune_request_t tune;
    tune.rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
    tune.dsp_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
    tune.rf_freq = tx_params.tx_freq;
    tune.dsp_freq = tx_params.tx_dsp_freq;

    usrp_tx->set_tx_freq(tune);
    update_usrp_tx = 0;
  }

  // recreate the frame generator only if necessary
  if (recreate_fg) {
    pthread_mutex_lock(&CE_fftw_mutex);
    ofdmflexframegen_destroy(fg);
    fg = ofdmflexframegen_create(tx_params.numSubcarriers, tx_params.cp_len,
                                 tx_params.taper_len, tx_params.subcarrierAlloc,
                                 &tx_params.fgprops);
    pthread_mutex_unlock(&CE_fftw_mutex);
    recreate_fg = 0;
  }

  ofdmflexframegen_setprops(fg, &tx_params.fgprops);

  // make sure the frame generator buffer is appropriately sized
  fgbuffer_len = tx_params.numSubcarriers + tx_params.cp_len;
  fgbuffer = (std::complex<float> *)realloc(
      (void *)fgbuffer, fgbuffer_len * sizeof(std::complex<float>));

  // reset flag
  update_tx_flag = 0;

  // set update data rate flag
  update_tx_data_rate = 1;
}

void ExtensibleCognitiveRadio::transmit_frame(unsigned char *_header,
                                              unsigned char *_payload,
                                              unsigned int _payload_len) {
  pthread_mutex_lock(&tx_mutex);

  if (log_phy_tx_flag) {
    // printf("\nLogging transmit parameters\n\n");
    log_tx_parameters();
  }

  // set up the metadta flags
  metadata_tx.start_of_burst = true; // never SOB when continuous
  metadata_tx.end_of_burst = false;  //
  metadata_tx.has_time_spec = false; // set to false to send immediately
  // TODO: flush buffers

  // vector buffer to send data to device
  std::vector<std::complex<float>> usrp_buffer(fgbuffer_len);

  float tx_gain_soft_lin = powf(10.0f, tx_params.tx_gain_soft / 20.0f);

  tx_header[0] =
      (ExtensibleCognitiveRadio::DATA << 6) + ((frame_num >> 8) & 0x3f);
  tx_header[1] = (frame_num)&0xff;
  frame_num++;

  // assemble frame
  ofdmflexframegen_assemble(fg, _header, _payload, _payload_len);

  // generate a single OFDM frame
  bool last_symbol = false;
  unsigned int i;
  while (!last_symbol) {

    // generate symbol
    last_symbol = ofdmflexframegen_writesymbol(fg, fgbuffer);

    // if(last_symbol)
    //	metadata_tx.end_of_burst   = true;

    // copy symbol and apply gain
    for (i = 0; i < fgbuffer_len; i++)
      usrp_buffer[i] = fgbuffer[i] * tx_gain_soft_lin;

    // send samples to the device
    usrp_tx->get_device()->send(&usrp_buffer.front(), usrp_buffer.size(),
                                metadata_tx, uhd::io_type_t::COMPLEX_FLOAT32,
                                uhd::device::SEND_MODE_FULL_BUFF);

    metadata_tx.start_of_burst = false; // never SOB when continuou

  } // while loop

  // send a few extra samples to the device
  // NOTE: this seems necessary to preserve last OFDM symbol in
  //       frame from corruption
  usrp_tx->get_device()->send(&usrp_buffer.front(), usrp_buffer.size(),
                              metadata_tx, uhd::io_type_t::COMPLEX_FLOAT32,
                              uhd::device::SEND_MODE_FULL_BUFF);

  // send a mini EOB packet
  metadata_tx.end_of_burst = true;

  usrp_tx->get_device()->send("", 0, metadata_tx,
                              uhd::io_type_t::COMPLEX_FLOAT32,
                              uhd::device::SEND_MODE_FULL_BUFF);
  pthread_mutex_unlock(&tx_mutex);
}

/////////////////////////////////////////////////////////////////////
// Receiver methods
/////////////////////////////////////////////////////////////////////

// set receiver frequency
void ExtensibleCognitiveRadio::set_rx_freq(double _rx_freq) {
  rx_params.rx_freq = _rx_freq;
  rx_params.rx_dsp_freq = 0.0;
  update_rx_flag = 1;
  update_usrp_rx = 1;
}

// set receiver frequency
void ExtensibleCognitiveRadio::set_rx_freq(double _rx_freq, double _dsp_freq) {
  rx_params.rx_freq = _rx_freq;
  rx_params.rx_dsp_freq = _dsp_freq;
  update_rx_flag = 1;
  update_usrp_rx = 1;
}

// get receiver state
double ExtensibleCognitiveRadio::get_rx_state() {
  return rx_state;
}

// get receiver frequency
double ExtensibleCognitiveRadio::get_rx_freq() {
  return rx_params.rx_freq - rx_params.rx_dsp_freq;
}

// set receiver sample rate
void ExtensibleCognitiveRadio::set_rx_rate(double _rx_rate) {
  rx_params.rx_rate = _rx_rate;
  update_rx_flag = 1;
  update_usrp_rx = 1;
}

// get receiver sample rate
double ExtensibleCognitiveRadio::get_rx_rate() { return rx_params.rx_rate; }

// set receiver hardware (UHD) gain
void ExtensibleCognitiveRadio::set_rx_gain_uhd(double _rx_gain_uhd) {
  rx_params.rx_gain_uhd = _rx_gain_uhd;
  update_rx_flag = 1;
  update_usrp_rx = 1;
}

// get receiver hardware (UHD) gain
double ExtensibleCognitiveRadio::get_rx_gain_uhd() {
  return rx_params.rx_gain_uhd;
}

// set receiver antenna
void ExtensibleCognitiveRadio::set_rx_antenna(char *_rx_antenna) {
  usrp_rx->set_rx_antenna(_rx_antenna);
}

// reset receiver objects and buffers
void ExtensibleCognitiveRadio::reset_rx() { ofdmflexframesync_reset(fs); }

// set number of subcarriers
void ExtensibleCognitiveRadio::set_rx_subcarriers(
    unsigned int _numSubcarriers) {
  // stop rx, destroy frame sync, set subcarriers, recreate frame sync
  rx_params.numSubcarriers = _numSubcarriers;
  update_rx_flag = 1;
  recreate_fs = 1;
}

// get number of subcarriers
unsigned int ExtensibleCognitiveRadio::get_rx_subcarriers() {
  return rx_params.numSubcarriers;
}

// set subcarrier allocation
void ExtensibleCognitiveRadio::set_rx_subcarrier_alloc(char *_subcarrierAlloc) {

  if (_subcarrierAlloc) {
    if (rx_params.subcarrierAlloc == NULL) {
      rx_params.subcarrierAlloc =
          (unsigned char *)malloc(rx_params.numSubcarriers);
    } else {
      rx_params.subcarrierAlloc = (unsigned char *)realloc(
          (void *)rx_params.subcarrierAlloc, rx_params.numSubcarriers);
    }
    memcpy(rx_params.subcarrierAlloc, _subcarrierAlloc,
           rx_params.numSubcarriers);
  } else {
    free(rx_params.subcarrierAlloc);
    rx_params.subcarrierAlloc = NULL;
  }

  update_rx_flag = 1;
  recreate_fs = 1;
}

// get subcarrier allocation
void ExtensibleCognitiveRadio::get_rx_subcarrier_alloc(char *subcarrierAlloc) {
  memcpy(subcarrierAlloc, rx_params.subcarrierAlloc, rx_params.numSubcarriers);
}

// set cp_len
void ExtensibleCognitiveRadio::set_rx_cp_len(unsigned int _cp_len) {
  // destroy frame gen, set cp length, recreate frame gen
  // pthread_mutex_lock(&rx_mutex);
  // ofdmflexframesync_destroy(fs);
  rx_params.cp_len = _cp_len;
  // fs = ofdmflexframesync_create(rx_params.numSubcarriers, rx_params.cp_len,
  //                              rx_params.taper_len,
  //                              rx_params.subcarrierAlloc,
  //                              rxCallback, (void *)this);
  // pthread_mutex_unlock(&rx_mutex);
}

// get cp_len
unsigned int ExtensibleCognitiveRadio::get_rx_cp_len() {
  return rx_params.cp_len;
}

// set taper_len
void ExtensibleCognitiveRadio::set_rx_taper_len(unsigned int _taper_len) {
  // destroy frame gen, set cp length, recreate frame gen
  // pthread_mutex_lock(&rx_mutex);
  // ofdmflexframesync_destroy(fs);
  rx_params.taper_len = _taper_len;
  // fs = ofdmflexframesync_create(rx_params.numSubcarriers, rx_params.cp_len,
  //                              rx_params.taper_len,
  //                              rx_params.subcarrierAlloc,
  //                              rxCallback, (void *)this);
  // pthread_mutex_unlock(&rx_mutex);
}

// get taper_len
unsigned int ExtensibleCognitiveRadio::get_rx_taper_len() {
  return rx_params.taper_len;
}

// set the period of time considered in the calculation of average rx statistics
void ExtensibleCognitiveRadio::set_rx_stat_tracking(bool state, float sec){
  rx_stat_tracking = state;
  rx_stat_tracking_period = sec;
}

float ExtensibleCognitiveRadio::get_rx_stat_tracking_period(){
  float rv;
  if(rx_stat_tracking)
    rv = rx_stat_tracking_period;
  else
    rv = 0.0;

  return rv;
}

struct ExtensibleCognitiveRadio::rx_statistics ExtensibleCognitiveRadio::get_rx_stats(){
  return rx_stats;
}

// start receiver
void ExtensibleCognitiveRadio::start_rx() {
  // set rx running flag
  rx_state = RX_CONTINUOUS;

  // tell device to start
  usrp_rx->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);

  // signal condition (tell rx worker to start)
  pthread_cond_signal(&rx_cond);
}

// stop receiver
void ExtensibleCognitiveRadio::stop_rx() {
  // set rx running flag
  rx_state = RX_STOPPED;

  // tell device to stop
  usrp_rx->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
}

// update the actual parameters being used by the transmitter
void ExtensibleCognitiveRadio::update_rx_params() {

  // update the USRP
  if (update_usrp_rx) {
    printf("Updating USRP parameters\n");
    usrp_rx->set_rx_gain(rx_params.rx_gain_uhd);
    usrp_rx->set_rx_rate(rx_params.rx_rate);

    uhd::tune_request_t tune;
    tune.rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
    tune.dsp_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
    tune.rf_freq = rx_params.rx_freq;
    tune.dsp_freq = rx_params.rx_dsp_freq;

    usrp_rx->set_rx_freq(tune);

    update_usrp_rx = 0;
  }

  // recreate the frame synchronizer only if necessary
  if (recreate_fs) {
    pthread_mutex_lock(&CE_fftw_mutex);
    ofdmflexframesync_destroy(fs);
    fs = ofdmflexframesync_create(
        rx_params.numSubcarriers, rx_params.cp_len, rx_params.taper_len,
        rx_params.subcarrierAlloc, rxCallback, (void *)this);
    pthread_mutex_unlock(&CE_fftw_mutex);
    recreate_fs = 0;
  }

  // reset flag
  update_rx_flag = 0;
}

// receiver worker thread
void *ECR_rx_worker(void *_arg) {
  // type cast input argument as ofdmtxrx object
  ExtensibleCognitiveRadio *ECR = (ExtensibleCognitiveRadio *)_arg;

  // set up receive buffer
  const size_t max_samps_per_packet =
      ECR->usrp_rx->get_device()->get_max_recv_samps_per_packet();
  ECR->ce_usrp_rx_buffer_length = max_samps_per_packet;

  std::complex<float> *buffer;
  buffer = (std::complex<float> *)malloc(max_samps_per_packet *
                                         sizeof(std::complex<float>));
  ECR->ce_usrp_rx_buffer = (std::complex<float> *)malloc(
      max_samps_per_packet * sizeof(std::complex<float>));

  while (ECR->rx_thread_running) {
    // wait for signal to start; lock mutex
    pthread_mutex_lock(&(ECR->rx_mutex));

    // this function unlocks the mutex and waits for the condition;
    // once the condition is set, the mutex is again locked
    pthread_cond_wait(&(ECR->rx_cond), &(ECR->rx_mutex));

    // unlock the mutex
    pthread_mutex_unlock(&(ECR->rx_mutex));
    // condition given; check state: run or exit
    if (!ECR->rx_thread_running) {
      dprintf("rx_worker finished\n");
      break;
    }

    // run receiver
    while (ECR->rx_state == RX_CONTINUOUS) {

      // dprintf("rx_running\n");
      // grab data from device
      size_t num_rx_samps = 0;
      num_rx_samps = ECR->usrp_rx->get_device()->recv(
          buffer, max_samps_per_packet, ECR->metadata_rx,
          uhd::io_type_t::COMPLEX_FLOAT32, uhd::device::RECV_MODE_ONE_PACKET);

      if (ECR->update_rx_flag)
        ECR->update_rx_params();

      // push resulting samples through synchronizer
      pthread_mutex_lock(&(ECR->rx_mutex));
      ofdmflexframesync_execute(ECR->fs, buffer, num_rx_samps);
      pthread_mutex_unlock(&(ECR->rx_mutex));

      if (ECR->ce_sensing_flag) {
        dprintf("Sensing\n");
        pthread_mutex_lock(&ECR->CE_mutex);
        // recheck flag incase it was unset by the CE while waiting to lock
        // mutex
        if (ECR->ce_sensing_flag) {
          // copy usrp sample buffer
          memcpy(ECR->ce_usrp_rx_buffer, buffer,
                 max_samps_per_packet * sizeof(std::complex<float>));

          // signal CE
          ECR->CE_metrics.CE_event = ExtensibleCognitiveRadio::USRP_RX_SAMPS;
          pthread_cond_signal(&ECR->CE_execute_sig);
        }
        pthread_mutex_unlock(&ECR->CE_mutex);
      }

      // check for uhd msg
      switch (ECR->uhd_msg) {
      case 1:
        // Signal CE thread
        pthread_mutex_lock(&ECR->CE_mutex);
        ECR->CE_metrics.CE_event = ExtensibleCognitiveRadio::UHD_OVERFLOW;
        pthread_cond_signal(&ECR->CE_execute_sig);
        pthread_mutex_unlock(&ECR->CE_mutex);
        ECR->uhd_msg = 0;
        break;
      case 2:
        // Signal CE thread
        pthread_mutex_lock(&ECR->CE_mutex);
        ECR->CE_metrics.CE_event = ExtensibleCognitiveRadio::UHD_UNDERRUN;
        pthread_cond_signal(&ECR->CE_execute_sig);
        pthread_mutex_unlock(&ECR->CE_mutex);
        ECR->uhd_msg = 0;
        break;
      case 0:
        break;
      }

    } // while rx_running
    dprintf("rx_worker finished running\n");

  } // while true

  free(buffer);
  free(ECR->ce_usrp_rx_buffer);
  if (ECR->CE_metrics.payload != NULL)
    free(ECR->CE_metrics.payload);

  dprintf("rx_worker exiting thread\n");
  pthread_exit(NULL);
}

// function to handle frames received by the ECR object
int rxCallback(unsigned char *_header, int _header_valid,
               unsigned char *_payload, unsigned int _payload_len,
               int _payload_valid, framesyncstats_s _stats, void *_userdata) {
  // typecast user argument as ECR object
  ExtensibleCognitiveRadio *ECR = (ExtensibleCognitiveRadio *)_userdata;

  // Store metrics and signal CE thread if using PHY layer metrics
  if (ECR->ce_phy_events) {
    ECR->CE_metrics.control_valid = _header_valid;
    memcpy(ECR->CE_metrics.control_info, _header + 2, 6);
    ECR->CE_metrics.frame_num = ((_header[0] & 0x3F) << 8 | _header[1]);
    ECR->CE_metrics.stats = _stats;
    ECR->CE_metrics.time_spec = ECR->metadata_rx.time_spec;
    ECR->CE_metrics.payload_valid = _payload_valid;
    //if (_payload_valid) {
      ECR->CE_metrics.payload_len = _payload_len;
      if (ECR->CE_metrics.payload != NULL)
        ECR->CE_metrics.payload = (unsigned char *)realloc(
            ECR->CE_metrics.payload, _payload_len * sizeof(unsigned char));
      else {
        ECR->CE_metrics.payload =
            (unsigned char *)malloc(_payload_len * sizeof(unsigned char));
      }
      memcpy(ECR->CE_metrics.payload, _payload,
             _payload_len * sizeof(unsigned char));
      ECR->CE_metrics.payload_len = _payload_len;
    //} else {
    //  ECR->CE_metrics.payload_len = 0;
    //  ECR->CE_metrics.payload = NULL;
    //}

    // Signal CE thread
    pthread_mutex_lock(&ECR->CE_mutex);
    ECR->CE_metrics.CE_event = ExtensibleCognitiveRadio::PHY; // set event type
                                                              // to phy once
                                                              // mutex is locked
    if (_header_valid) {
      if (ExtensibleCognitiveRadio::DATA == ((_header[0] >> 6) & 0x3))
        ECR->CE_metrics.CE_frame = ExtensibleCognitiveRadio::DATA;
      else
        ECR->CE_metrics.CE_frame = ExtensibleCognitiveRadio::CONTROL;
    } else {
      ECR->CE_metrics.CE_frame = ExtensibleCognitiveRadio::UNKNOWN;
    }
    pthread_cond_signal(&ECR->CE_execute_sig);
    pthread_mutex_unlock(&ECR->CE_mutex);

    // Print metrics if required
    if (ECR->print_metrics_flag)
      ECR->print_metrics(ECR);
    
    // Log metrics locally if required
    if (ECR->log_phy_rx_flag)
      ECR->log_rx_metrics();

    // Track statistics if required
    if (ECR->rx_stat_tracking)
      ECR->update_rx_stats();
  }

  //char payload[_payload_len];
  //memcpy(payload, _payload, _payload_len * sizeof(unsigned char));

  int nwrite = 0;
  if (_payload_valid) {
    if (ExtensibleCognitiveRadio::DATA == ((_header[0] >> 6) & 0x3)) {
      // Pass payload to tun interface
      dprintf("Received payload length: %u\n", _payload_len);
      for (unsigned int i = 0; i < (_payload_len / 288); i++) {
        nwrite = cwrite(ECR->tunfd, (char*)&_payload[i * 288], 288);
        if (nwrite != 288)
          printf("Number of bytes written to TUN interface not equal to the "
                 "expected amount\n");
      }
    }
  }

  return 0;
}

void ExtensibleCognitiveRadio::update_rx_stats(){
  // static variables only needed by this function to track statistics
  static std::vector<struct timeval> time_stamp;
  static std::vector<float> evm;
  static std::vector<float> rssi;
  static std::vector<int> payload_valid;
  static std::vector<int> payload_len;
  static std::vector<int> bit_errors;
  
  static float sum_evm = 0.0;
  static float sum_rssi = 0.0;
  static int sum_payload_valid = 0;
  static int sum_payload_len = 0;
  static int sum_bit_errors = 0;
  static int sum_valid_bytes = 0;

  static int N = 0; // number of valid data points (frames received)
  static int K = 0; // 
  static int ind_first = 0;
  static int ind_last;

  // variables to keep track of known and unknown portions of packets
  static const int total_frame_len = 32 + CRTS_CR_PACKET_LEN;
  static const int unknown_len = 32 + CRTS_CR_PACKET_NUM_LEN;
  static const float overhead = (float)unknown_len/(float)total_frame_len;

  // initial allocation of memory vectors
  if (K ==0){
    K = 128;
    time_stamp.resize(K);
    evm.resize(K,0.0);
    rssi.resize(K,0.0);
    payload_valid.resize(K,0);
    payload_len.resize(K,0);
    bit_errors.resize(K,0);
  }

  // update tracking period since it can be modified
  struct timeval stat_tracking_period;
  stat_tracking_period.tv_sec = (time_t)rx_stat_tracking_period;
  stat_tracking_period.tv_usec = (suseconds_t)(fmod(rx_stat_tracking_period,1.0)*1e6);

  // compute timeval for earliest time that will be considered in the statistics
  struct timeval time_now;
  gettimeofday(&time_now, NULL);
  struct timeval begin_time;
  timersub(&time_now, &stat_tracking_period, &begin_time);

  // remove any old data points from consideration
  while(timercmp(&time_stamp[ind_first], &begin_time, <) && N>0){
    sum_evm -= evm[ind_first];
    sum_rssi -= rssi[ind_first];
    sum_payload_valid -= payload_valid[ind_first];
    sum_payload_len -= payload_len[ind_first]; 
    sum_valid_bytes -= payload_valid[ind_first]*payload_len[ind_first]; 
    sum_bit_errors -= bit_errors[ind_first];
    N--;

    if (ind_first == K-1)
      ind_first = 0;
    else
      ind_first++;
  }

  // resize memory if needed
  N++;
  if (N>K) {
    K *= 2;
    time_stamp.resize(K);
    evm.resize(K,0.0);
    rssi.resize(K,0.0);
    payload_valid.resize(K,0);
    payload_len.resize(K,0);
    bit_errors.resize(K,0);
  }

  // update the last data point index
  ind_last = ind_first+(N-1);
  if(ind_last >= K)
    ind_last -= K;

  // calculate number of bit errors if needed
  unsigned int errors = 0;
  printf("Header valid: %i\n", CE_metrics.control_valid);
  printf("%i byte payload valid: %i\n", CE_metrics.payload_len, CE_metrics.payload_valid);
  if(!CE_metrics.payload_valid) {
    printf("Calculating errors of a payload length %i\n", CE_metrics.payload_len);
    for(unsigned int i=0; i<CE_metrics.payload_len; i++) {
      // only count errors in the known portion (not the IP header and packet number)
      if(i%(total_frame_len) >= unknown_len) {
        /*printf("\nRX: %c, known: %c, xor: %u, errs: %u", 
          CE_metrics.payload[i], 
          known_net_payload[(i%total_frame_len)-unknown_len],
          (unsigned int) CE_metrics.payload[i]^known_net_payload[(i%total_frame_len)-unknown_len],
          (unsigned int) std::bitset<8>((CE_metrics.payload[i]^known_net_payload[(i%total_frame_len)-unknown_len])).count()
          );*/
        errors += std::bitset<8>((CE_metrics.payload[i]^known_net_payload[(i%total_frame_len)-unknown_len])).count();
      }
    }
  }

  // store latest values
  time_stamp[ind_last] = time_now;
  evm[ind_last] = pow(10.0,CE_metrics.stats.evm/10.0);
  rssi[ind_last] = pow(10.0,CE_metrics.stats.rssi/10.0);
  payload_valid[ind_last] = CE_metrics.payload_valid;
  payload_len[ind_last] = CE_metrics.payload_len;
  bit_errors[ind_last] = errors;

  // update sums
  sum_evm += evm[ind_last];
  sum_rssi += rssi[ind_last];
  sum_payload_valid += payload_valid[ind_last];
  sum_payload_len += payload_len[ind_last];
  sum_valid_bytes += payload_valid[ind_last]*payload_len[ind_last];
  sum_bit_errors += bit_errors[ind_last];

  // update average values
  rx_stats.frames_received = N;
  rx_stats.avg_evm = 10.0*log10(sum_evm/(float)N);
  rx_stats.avg_rssi = 10.0*log10(sum_rssi/(float)N);
  rx_stats.avg_per = (float)(N-sum_payload_valid)/(float)N;
  rx_stats.avg_throughput = 8.0*(float)sum_valid_bytes/rx_stat_tracking_period;
  rx_stats.avg_ber = (1.0+overhead)*(float)sum_bit_errors/(8.0*sum_payload_len);

  //for(int i=offset; i< payload_len[ind_last]; i++)
  //  printf("%c", CE_metrics.payload[i]);
  //printf("\n");

  printf("Payload valid: %i\n", CE_metrics.payload_valid);
  printf("bit errors: %i\n", bit_errors[ind_last]);
  printf("sum bit errors: %i\n", sum_bit_errors);
  printf("sum payload_len: %i\n", sum_payload_len);
  printf("BER: %f\n\n", rx_stats.avg_ber);
  
  /*printf("\n");
  printf("N: %i\n", N);
  printf("K:, %i\n", K);
  printf("evm: %f", evm[ind_first]);
  for(int i=0; i<N-1; i++){
    int ind = ind_first + i;
    if (ind > K)
      ind -= K;
    printf(", %f", evm[ind]);
  }
  printf("\navg evm: %f\n", sum_evm/(float)N);
  printf("rssi: %f", rssi[ind_first]);
  for(int i=0; i<N-1; i++){
    int ind = ind_first + i;
    if (ind > K)
      ind -= K;
    printf(", %f", rssi[ind]);
  }
  printf("\navg rssi: %f\n", sum_rssi/(float)N);
  printf("ind_first: %i\n", ind_first);
  printf("ind_last: %i\n", ind_last);
  printf("valid: %i", payload_valid[ind_first]);
  for(int i=1; i<N; i++){
    int ind = ind_first + i;
    if (ind >= K)
      ind -= K;
    printf(", %i", payload_valid[ind]);
  }
  printf("\nsum valid: %i\n", sum_payload_valid);
  printf("avg per: %f\n", (float)(N-sum_payload_valid)/(float)N);
  printf("length: %i", payload_len[ind_first]);
  for(int i=1; i<N; i++){
    int ind = ind_first + i;
    if (ind >= K)
      ind -= K;
    printf(", %i", payload_len[ind]);
  }
  printf("\nthroughput: %f\n", 8.0*(float)sum_payload_len/(float)rx_stat_tracking_period);
  */
}

// transmitter worker thread
void *ECR_tx_worker(void *_arg) {
  // type cast input argument as ECR object
  ExtensibleCognitiveRadio *ECR = (ExtensibleCognitiveRadio *)_arg;

  // set up transmit buffer
  int buffer_len = 256 * 8 * 2;
  unsigned char buffer[buffer_len];
  unsigned char *payload;
  unsigned int payload_len;
  int nread;
  int tx_frame_counter;

  while (ECR->tx_thread_running) {
    pthread_mutex_lock(&(ECR->tx_mutex));
    // wait for signal to start
    pthread_cond_wait(&(ECR->tx_cond), &(ECR->tx_mutex));
    // unlock the mutex
    pthread_mutex_unlock(&(ECR->tx_mutex));

    memset(buffer, 0, buffer_len);

    fd_set fds;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;

    // variables to keep track of number of frames if needeed
    tx_frame_counter = 0;
    
    // run transmitter
    while (ECR->tx_state != TX_STOPPED) {

      if (ECR->update_tx_flag) {
        ECR->update_tx_params();
      }

      nread = 0;
      int bps = modulation_types[ECR->tx_params.fgprops.mod_scheme].bps;
      int payload_sym_length = ECR->tx_params.payload_sym_length;
      int payload_byte_length =
          (int)ceilf((float)(payload_sym_length * bps) / 8.0);

      while (nread < payload_byte_length && (ECR->tx_state != TX_STOPPED)) {
        FD_ZERO(&fds);
        FD_SET(ECR->tunfd, &fds);

        // check if anything is available on the TUN interface
        if (select(ECR->tunfd + 1, &fds, NULL, NULL, &timeout) > 0) {

          // grab data from TUN interface
          nread += cread(ECR->tunfd, (char *)(&buffer[nread]), buffer_len);
          if (nread < 0) {
            printf("Error reading from interface");
            close(ECR->tunfd);
            exit(1);
          }
        }
      }

      payload = buffer;
      payload_len = nread;

      // transmit frame
      tx_frame_counter++;
      ECR->transmit_frame(ECR->tx_header, payload, payload_len);

      // change state to stopped once all frames have been transmitted
      if ((ECR->tx_state == TX_FOR_FRAMES) &&
          (tx_frame_counter >= ECR->num_tx_frames)) {
        ECR->tx_state = TX_STOPPED;
      }
    } // while tx_running
    
    // signal CE that transmission has finished
    pthread_mutex_lock(&ECR->CE_mutex);
    ECR->CE_metrics.CE_event = ExtensibleCognitiveRadio::TX_COMPLETE;
    pthread_cond_signal(&ECR->CE_execute_sig);
    pthread_mutex_unlock(&ECR->CE_mutex);

    dprintf("tx_worker finished running\n");
  } // while tx_thread_running
  dprintf("tx_worker exiting thread\n");
  pthread_exit(NULL);
}

// main loop of CE
void *ECR_ce_worker(void *_arg) {
  ExtensibleCognitiveRadio *ECR = (ExtensibleCognitiveRadio *)_arg;

  struct timeval time_now;
  double timeout_ns;
  double timeout_spart;
  double timeout_nspart;
  struct timespec timeout;

  // until CE thread is joined
  while (ECR->ce_thread_running) {

    pthread_mutex_lock(&ECR->CE_mutex);
    pthread_cond_wait(&ECR->CE_cond, &ECR->CE_mutex);
    pthread_mutex_unlock(&ECR->CE_mutex);

    while (ECR->ce_running) {

      // Get current time of day
      gettimeofday(&time_now, NULL);

      // Calculate timeout time in nanoseconds
      timeout_ns = (double)time_now.tv_usec * 1e3 +
                   (double)time_now.tv_sec * 1e9 + ECR->ce_timeout_ms * 1e6;
      // Convert timeout time to s and ns parts
      timeout_nspart = modf(timeout_ns / 1e9, &timeout_spart);
      // Put timeout into timespec struct
      timeout.tv_sec = (long int)timeout_spart;
      timeout.tv_nsec = (long int)(timeout_nspart * 1e9);

      // Wait for signal from receiver
      pthread_mutex_lock(&ECR->CE_mutex);
      if (ETIMEDOUT == pthread_cond_timedwait(&ECR->CE_execute_sig,
                                              &ECR->CE_mutex, &timeout))
        ECR->CE_metrics.CE_event = ExtensibleCognitiveRadio::TIMEOUT;

      // execute CE
      ECR->CE->execute(ECR);
      pthread_mutex_unlock(&ECR->CE_mutex);
    }
  }
  dprintf("ce_worker exiting thread\n");
  pthread_exit(NULL);
}

///////////////////////////////////////////////////////////////////////
// Print/log methods
///////////////////////////////////////////////////////////////////////

void ExtensibleCognitiveRadio::print_metrics(ExtensibleCognitiveRadio *ECR) {
  printf("\n---------------------------------------------------------\n");
  if (ECR->CE_metrics.control_valid)
    printf("Received Frame %u metrics:      Received Frame Parameters:\n",
           ECR->CE_metrics.frame_num);
  else
    printf("Received Frame ? metrics:      Received Frame Parameters:\n");
  printf("---------------------------------------------------------\n");
  printf("Control Valid:    %-6i      Modulation Scheme:   %s\n",
         ECR->CE_metrics.control_valid,
         modulation_types[ECR->CE_metrics.stats.mod_scheme].name);
  // See liquid soruce: src/modem/src/modem_utilities.c
  // for definition of modulation_types
  printf("Payload Valid:    %-6i      Modulation bits/sym: %u\n",
         ECR->CE_metrics.payload_valid, ECR->CE_metrics.stats.mod_bps);
  printf("EVM:              %-8.2f    Check:               %s\n",
         ECR->CE_metrics.stats.evm,
         crc_scheme_str[ECR->CE_metrics.stats.check][0]);
  // See liquid source: src/fec/src/crc.c
  // for definition of crc_scheme_str
  printf("RSSI:             %-8.2f    Inner FEC:           %s\n",
         ECR->CE_metrics.stats.rssi,
         fec_scheme_str[ECR->CE_metrics.stats.fec0][0]);
  printf("Frequency Offset: %-8.2f    Outter FEC:          %s\n",
         ECR->CE_metrics.stats.cfo,
         fec_scheme_str[ECR->CE_metrics.stats.fec1][0]);
  // See liquid soruce: src/fec/src/fec.c
  // for definition of fec_scheme_str
}

void ExtensibleCognitiveRadio::log_rx_metrics() {

  // open file, append metrics, and close
  if (log_rx_fstream.is_open()) {
    log_rx_fstream.write((char *)&CE_metrics, sizeof(struct metric_s));
    log_rx_fstream.write((char *)&rx_params, sizeof(struct rx_parameter_s));
  }
}

void ExtensibleCognitiveRadio::log_tx_parameters() {

  // update current time
  struct timeval tv;
  gettimeofday(&tv, NULL);

  // open file, append parameters, and close
  if (log_tx_fstream.is_open()) {
    log_tx_fstream.write((char *)&tv, sizeof(tv));
    log_tx_fstream.write((char *)&tx_params, sizeof(struct tx_parameter_s));
  }
}

void ExtensibleCognitiveRadio::reset_log_files() {

  if (log_phy_rx_flag) {
    log_rx_fstream.open(phy_rx_log_file,
                        std::ofstream::out | std::ofstream::trunc);
    if (!log_rx_fstream.is_open()) {
      printf("Error opening rx log file: %s\n", phy_rx_log_file);
      exit(1);
    }
  }

  if (log_phy_tx_flag) {
    log_tx_fstream.open(phy_tx_log_file,
                        std::ofstream::out | std::ofstream::trunc);
    if (!log_tx_fstream.is_open()) {
      printf("Error opening tx log file: %s\n", phy_tx_log_file);
      exit(1);
    }
  }
}
