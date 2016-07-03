#ifndef _SC_RX_OVERFLOW_TEST_
#define _SC_RX_OVERFLOW_TEST_

#include "scenario_controller.hpp"

#define RX_OVERFLOW_LOG_FILE ("logs/csv/rx_overflow_log.csv")
#define DEFAULT_DWELL_TIME_S 60
#define DEFAULT_SETTLE_TIME_S 2

class SC_Rx_Overflow_Test : public ScenarioController {

private:

  int debug_level;
  FILE *log;
  struct ExtensibleCognitiveRadio::rx_statistics rx_stats;
  float dwell_time_s;
  float settle_time_s;
  bool node_1_feedback_received;
  bool node_2_feedback_received;

  int rate_ind;
  static int constexpr num_rates = 3;
  double rates[num_rates] = {1e6, 2e6, 5e6};
  
  int mod_ind;
  static int constexpr num_mods = 1;
  int mods[num_mods] = {LIQUID_MODEM_BPSK};/*,
                        LIQUID_MODEM_QPSK,
                        LIQUID_MODEM_QAM16,
                        LIQUID_MODEM_QAM64};*/
  
  int fec_ind;
  static int constexpr num_fecs = 3; //25;
  int fecs[num_fecs] = {LIQUID_FEC_NONE,
                        //LIQUID_FEC_HAMMING74,
                        //LIQUID_FEC_HAMMING84,
                        //LIQUID_FEC_HAMMING128,
                        //LIQUID_FEC_GOLAY2412,
                        //LIQUID_FEC_SECDED2216,
                        //LIQUID_FEC_SECDED3932,
                        //LIQUID_FEC_SECDED7264,
                        LIQUID_FEC_CONV_V27,
                        LIQUID_FEC_CONV_V29/*,
                        LIQUID_FEC_CONV_V39,
                        LIQUID_FEC_CONV_V615,
                        LIQUID_FEC_CONV_V27P23,
                        LIQUID_FEC_CONV_V27P34,
                        //LIQUID_FEC_CONV_V27P45,
                        //LIQUID_FEC_CONV_V27P56,
                        //LIQUID_FEC_CONV_V27P67,
                        LIQUID_FEC_CONV_V27P78,
                        LIQUID_FEC_CONV_V29P23,
                        LIQUID_FEC_CONV_V29P34,
                        //LIQUID_FEC_CONV_V29P45,
                        //LIQUID_FEC_CONV_V29P56,
                        //LIQUID_FEC_CONV_V29P67,
                        LIQUID_FEC_CONV_V29P78,
                        //LIQUID_FEC_RS_M8*/
                       };

  int uhd_overflows[2][num_rates][num_mods][num_fecs];

public:
  SC_Rx_Overflow_Test(int argc, char **argv);
  ~SC_Rx_Overflow_Test();
  virtual void execute();
  virtual void initialize_node_fb();
};

#endif
