#include <stdio.h>
#include <limits.h>
#include "ECR.hpp"

void help_logs2python() {
  printf("logs2python -- Create Python .py file containing log data.\n");
  printf("               The new file can then be imported into a Python script to process the data.\n");
  printf(" -h : Help.\n");
  printf(" -l : Name of log file to process (required).\n");
  printf(" -r : Log file contains PHY receive metrics.\n");
  printf(" -t : Log file contains PHY transmit parameters.\n");
  printf(" -i : Log file contains interferer transmit parameters.\n");
  printf(" -c : Log file contains NET receive data.\n");
  printf(" -N : Total number of repetitions for this scenario (default: 1)\n");
  printf(" -n : The repetition number for this scenario (required if -N is given)\n");
}

int main(int argc, char ** argv){

  char log_file[PATH_MAX]; 
  char output_file[PATH_MAX];
  char multi_file[PATH_MAX];

  enum log_t { PHY_RX = 0, PHY_TX, INT_TX, NET_RX };

  // default log type is PHY RX metrics
  log_t log_type = PHY_RX;

  strcpy(log_file, "logs/bin/");
  strcpy(output_file, "logs/python/");
  strcpy(multi_file, "logs/python/");

  unsigned int totalNumReps = 1;
  unsigned int repNumber = 1;

  // Option flags
  int l_opt = 0;
  int n_opt = 0;

  int d;
  while((d = getopt(argc, argv, "hl:rticN:n:")) != EOF){
    switch(d){
      case 'h': 
        help_logs2python();
        return 0;
      case 'l': {
        strcat(log_file, optarg);
        strcat(log_file, ".log");
        strcat(output_file, optarg);
        strcat(output_file, ".py");
        strcat(multi_file, optarg);
        char *ptr_ = strrchr(multi_file, (int) '_');
        if (ptr_)
            multi_file[ ptr_ - multi_file] = '\0';
        strcat(multi_file, ".py");
        l_opt = 1;
        break;
        }
      case 'r':
        break;
      case 't': 
        log_type = PHY_TX;
        break;
      case 'i': 
        log_type = INT_TX;
        break;
      case 'c': 
        log_type = NET_RX;
        break;
      case 'N': 
        totalNumReps = atoi(optarg);
        break;
      case 'n': 
        repNumber = atoi(optarg);
        n_opt = 1;
        break;
    }
  }

  // Check that log file and output file names were given
  if (!l_opt)
  {
    printf("Please give -l option.\n\n");
    help_logs2python();
    return 1;
  }
  // Check that -n option is given as necessary
  if (totalNumReps>1 && !n_opt)
  {
    printf("-n option is required whenever -N option is given");
    help_logs2python();
    return 1;
  }

  printf("Log file name: %s\n", log_file);
  printf("Output file name: %s\n", output_file);

  FILE * file_in = fopen(log_file, "rb");
  FILE * file_out;
  if (totalNumReps==1)
  {
      file_out = fopen(output_file, "w");
  }
  else
  {
      file_out = fopen(multi_file, "a");
  }

  struct ExtensibleCognitiveRadio::metric_s metrics = {};
  struct ExtensibleCognitiveRadio::rx_parameter_s rx_params = {};
  struct ExtensibleCognitiveRadio::tx_parameter_s tx_params = {};
  int i = 1;

  switch(log_type)
  {
    case PHY_RX:
    {
      fprintf(file_out,   "phy_rx_t               = list()\n");
      // metrics
      fprintf(file_out,   "phy_rx_Control_valid   = list()\n");
      fprintf(file_out,   "phy_rx_Payload_valid   = list()\n");
      fprintf(file_out,   "phy_rx_EVM             = list()\n");
      fprintf(file_out,   "phy_rx_RSSI            = list()\n");
      fprintf(file_out,   "phy_rx_CFO             = list()\n");
      fprintf(file_out,   "phy_rx_num_syms        = list()\n");    
      fprintf(file_out,   "phy_rx_mod_scheme      = list()\n");
      fprintf(file_out,   "phy_rx_BPS             = list()\n");
      fprintf(file_out,   "phy_rx_fec0            = list()\n");
      fprintf(file_out,   "phy_rx_fec1            = list()\n");
      // parameters
      fprintf(file_out,   "phy_rx_numSubcarriers  = list()\n");
      fprintf(file_out,   "phy_rx_cp_len          = list()\n");
      fprintf(file_out,   "phy_rx_taper_len       = list()\n");
      fprintf(file_out,   "phy_rx_gain_uhd        = list()\n");
      fprintf(file_out,   "phy_rx_freq            = list()\n");
      fprintf(file_out,   "phy_rx_rate            = list()\n");

      while(fread((char*)&metrics, 
                  sizeof(struct ExtensibleCognitiveRadio::metric_s), 1,
                  file_in)) {
        fread((char*)&rx_params,
              sizeof(struct ExtensibleCognitiveRadio::rx_parameter_s), 1,
              file_in);
        fprintf(file_out, "phy_rx_t.append(%li + %f)\n",
                metrics.time_spec.get_full_secs(), 
                metrics.time_spec.get_frac_secs());
        //metrics
        fprintf(file_out, "phy_rx_Control_valid.append(%i)\n",
                metrics.control_valid);
        fprintf(file_out, "phy_rx_Payload_valid.append(%i)\n",
                metrics.payload_valid);
        fprintf(file_out, "phy_rx_EVM.append(%f)\n",
                metrics.stats.evm);
        fprintf(file_out, "phy_rx_RSSI.append(%f)\n",
                metrics.stats.rssi);
        fprintf(file_out, "phy_rx_CFO.append(%f)\n",
                metrics.stats.cfo);
        fprintf(file_out, "phy_rx_num_syms.append(%i)\n",
                metrics.stats.num_framesyms);    
        fprintf(file_out, "phy_rx_mod_scheme.append(%i)\n",
                metrics.stats.mod_scheme);
        fprintf(file_out, "phy_rx_BPS.append(%i)\n",
                metrics.stats.mod_bps);
        fprintf(file_out, "phy_rx_fec0.append(%i)\n",
                metrics.stats.fec0);
        fprintf(file_out, "phy_rx_fec1.append(%i)\n",
                metrics.stats.fec1);
        // parameters
        fprintf(file_out, "phy_rx_numSubcarriers.append(%u)\n",
                rx_params.numSubcarriers);
        fprintf(file_out, "phy_rx_cp_len.append(%u)\n",
                rx_params.cp_len);
        fprintf(file_out, "phy_rx_taper_len.append(%u)\n",
                rx_params.taper_len);
        fprintf(file_out, "phy_rx_gain_uhd.append(%f)\n",
                rx_params.rx_gain_uhd);
        fprintf(file_out, "phy_rx_freq.append(%f)\n",
                rx_params.rx_freq - rx_params.rx_dsp_freq);
        fprintf(file_out, "phy_rx_rate.append(%f)\n",
                rx_params.rx_rate);
        i++;
      }
      // If appending to the multiFile,
      // then put the data into the next elements of the multi array
      if (totalNumReps>1)
      {
        // Check that the multi array exists. Create it if it doesn't.
        fprintf(file_out, "if 'phy_rx_t_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out, "    phy_rx_t_all_repetitions = [None]*%u\n",
                totalNumReps);
        // metrics
        fprintf(file_out,
                "if 'phy_rx_Control_valid_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_Control_valid_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_Payload_valid_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_Payload_valid_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_EVM_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_EVM_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_RSSI_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_RSSI_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_CFO_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_CFO_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_num_syms_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_num_syms_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_mod_scheme_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_mod_scheme_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_BPS_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_BPS_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_fec0_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_fec0_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_fec1_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_fec1_all_repetitions = [None]*%u\n",
                totalNumReps);
        // parameters
        fprintf(file_out,
                "if 'phy_rx_numSubcarriers_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_numSubcarriers_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_cp_len_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_cp_len_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_taper_len_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_taper_len_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_gain_uhd_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_gain_uhd_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_freq_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_freq_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_rx_rate_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_rx_rate_all_repetitions = [None]*%u\n",
                totalNumReps);

        // Place data in multiarray
        fprintf(file_out,
                "phy_rx_t_all_repetitions[%u]     = phy_rx_t\n",
                repNumber-1);
        // metrics
        fprintf(file_out,
                "phy_rx_Control_valid_all_repetitions[%u] = phy_rx_Control_valid\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_Payload_valid_all_repetitions[%u] = phy_rx_Payload_valid\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_EVM_all_repetitions[%u]           = phy_rx_EVM\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_RSSI_all_repetitions[%u]          = phy_rx_RSSI\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_CFO_all_repetitions[%u]           = phy_rx_CFO\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_num_syms_all_repetitions[%u]      = phy_rx_num_syms\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_mod_scheme_all_repetitions[%u]    = phy_rx_mod_scheme\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_BPS_all_repetitions[%u]           = phy_rx_BPS\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_fec0_all_repetitions[%u]          = phy_rx_fec0\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_fec1_all_repetitions[%u]          = phy_rx_fec1\n",
                repNumber-1);
        // parameters
        fprintf(file_out,
                "phy_rx_numSubcarriers_all_repetitions[%u]= phy_rx_numSubcarriers\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_cp_len_all_repetitions[%u]    = phy_rx_cp_len\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_taper_len_all_repetitions[%u] = phy_rx_taper_len\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_gain_uhd_all_repetitions[%u]  = phy_rx_gain_uhd\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_freq_all_repetitions[%u]      = phy_rx_freq\n",
                repNumber-1);
        fprintf(file_out,
                "phy_rx_rate_all_repetitions[%u]      = phy_rx_rate\n",
                repNumber-1);

        // Delete redundant data
        fprintf(file_out, "del phy_rx_t\n");
        // metrics
        fprintf(file_out, "del phy_rx_Control_valid\n");
        fprintf(file_out, "del phy_rx_Payload_valid\n");
        fprintf(file_out, "del phy_rx_EVM\n");
        fprintf(file_out, "del phy_rx_RSSI\n");
        fprintf(file_out, "del phy_rx_CFO\n");
        fprintf(file_out, "del phy_rx_num_syms\n");
        fprintf(file_out, "del phy_rx_mod_scheme\n");
        fprintf(file_out, "del phy_rx_BPS\n");
        fprintf(file_out, "del phy_rx_fec0\n");
        fprintf(file_out, "del phy_rx_fec1\n\n");
        // parameters
        fprintf(file_out, "del phy_rx_numSubcarriers\n");
        fprintf(file_out, "del phy_rx_cp_len\n");
        fprintf(file_out, "del phy_rx_taper_len\n");
        fprintf(file_out, "del phy_rx_gain_uhd\n");
        fprintf(file_out, "del phy_rx_freq\n");
        fprintf(file_out, "del phy_rx_rate\n");
      }
      break;
    }
    // handle PHY TX log
    case PHY_TX:
    {
      fprintf(file_out, "phy_tx_t                 = list()\n");
      fprintf(file_out, "phy_tx_numSubcarriers    = list()\n");
      fprintf(file_out, "phy_tx_cp_len            = list()\n");
      fprintf(file_out, "phy_tx_taper_len         = list()\n");
      fprintf(file_out, "phy_tx_gain_uhd          = list()\n");
      fprintf(file_out, "phy_tx_gain_soft         = list()\n");
      fprintf(file_out, "phy_tx_freq              = list()\n");
      fprintf(file_out, "phy_tx_lo_freq           = list()\n");
      fprintf(file_out, "phy_tx_dsp_freq          = list()\n");
      fprintf(file_out, "phy_tx_rate              = list()\n");    
      fprintf(file_out, "phy_tx_mod_scheme        = list()\n");
      fprintf(file_out, "phy_tx_fec0              = list()\n");
      fprintf(file_out, "phy_tx_fec1              = list()\n\n");

      struct timeval log_time;
      while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1,
                  file_in)){
        fread((char*)&tx_params,
              sizeof(struct ExtensibleCognitiveRadio::tx_parameter_s), 1,
              file_in);
        fprintf(file_out, "phy_tx_t.append(%li + 1e-6*%li)\n",
                log_time.tv_sec, log_time.tv_usec);
        fprintf(file_out, "phy_tx_numSubcarriers.append(%u)\n",
                tx_params.numSubcarriers);
        fprintf(file_out, "phy_tx_cp_len.append(%u)\n",
                tx_params.cp_len);
        fprintf(file_out, "phy_tx_taper_len.append(%u)\n",
                tx_params.taper_len);
        fprintf(file_out, "phy_tx_gain_uhd.append(%f)\n",
                tx_params.tx_gain_uhd);
        fprintf(file_out, "phy_tx_gain_soft.append(%f)\n",
                tx_params.tx_gain_soft);
        fprintf(file_out, "phy_tx_freq.append(%f)\n",
                tx_params.tx_freq + tx_params.tx_dsp_freq);
        fprintf(file_out, "phy_tx_lo_freq.append(%f)\n",
                tx_params.tx_freq);
        fprintf(file_out, "phy_tx_dsp_freq.append(%f)\n",
                tx_params.tx_dsp_freq);
        fprintf(file_out, "phy_tx_rate.append(%f)\n",
                tx_params.tx_rate);    
        fprintf(file_out, "phy_tx_mod_scheme.append(%i)\n",
                tx_params.fgprops.mod_scheme);
        fprintf(file_out, "phy_tx_fec0.append(%i)\n",
                tx_params.fgprops.fec0);
        fprintf(file_out, "phy_tx_fec1.append(%i)\n\n",
                tx_params.fgprops.fec1);
        i++;
      }

      // If appending to the multiFile, then put the data into the next elements of the multi array
      if (totalNumReps>1)
      {
        // Check that the multi array exists. Create it if it doesn't.
        fprintf(file_out,
                "if 'phy_tx_t_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_t_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_tx_numSubcarriers_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_numSubcarriers_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_tx_cp_len_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_cp_len_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_tx_taper_len_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_taper_len_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_tx_gain_uhd_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_gain_uhd_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_tx_gain_soft_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_gain_soft_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_tx_freq_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_freq_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_tx_lo_freq_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_lo_freq_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_tx_dsp_freq_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_dsp_freq_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_tx_rate_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_rate_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_tx_mod_scheme_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_mod_scheme_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_tx_fec0_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_fec0_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'phy_tx_fec1_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    phy_tx_fec1_all_repetitions = [None]*%u\n",
                totalNumReps);

        // Place data in multiarray
        fprintf(file_out,
                "phy_tx_t_all_repetitions[%u] = phy_tx_t\n",
                repNumber-1);
        fprintf(file_out,
                "phy_tx_numSubcarriers_all_repetitions[%u] = phy_tx_numSubcarriers\n",
                repNumber-1);
        fprintf(file_out,
                "phy_tx_cp_len_all_repetitions[%u] = phy_tx_cp_len\n",
                repNumber-1);
        fprintf(file_out,
                "phy_tx_taper_len_all_repetitions[%u] = phy_tx_taper_len\n",
                repNumber-1);
        fprintf(file_out,
                "phy_tx_gain_uhd_all_repetitions[%u] = phy_tx_gain_uhd\n",
                repNumber-1);
        fprintf(file_out,
                "phy_tx_gain_soft_all_repetitions[%u] = phy_tx_gain_soft\n",
                repNumber-1);
        fprintf(file_out,
                "phy_tx_freq_all_repetitions[%u] = phy_tx_freq\n",
                repNumber-1);
        fprintf(file_out, 
                "phy_tx_lo_freq_all_repetitions[%u] = phy_tx_lo_freq\n",
                repNumber-1);
        fprintf(file_out, 
                "phy_tx_dsp_freq_all_repetitions[%u] = phy_tx_dsp_freq\n",
                repNumber-1);
        fprintf(file_out,
                "phy_tx_rate_all_repetitions[%u] = phy_tx_rate\n",
                repNumber-1);
        fprintf(file_out, 
                "phy_tx_mod_scheme_all_repetitions[%u] = phy_tx_mod_scheme\n",
                repNumber-1);
        fprintf(file_out, 
                "phy_tx_fec0_all_repetitions[%u] = phy_tx_fec0\n",
                repNumber-1);
        fprintf(file_out, 
                "phy_tx_fec1_all_repetitions[%u] = phy_tx_fec1\n\n",
                repNumber-1);

        // Delete redundant data
        fprintf(file_out, "del phy_tx_t\n");
        fprintf(file_out, "del phy_tx_numSubcarriers\n");
        fprintf(file_out, "del phy_tx_cp_len\n");
        fprintf(file_out, "del phy_tx_taper_len\n");
        fprintf(file_out, "del phy_tx_gain_uhd\n");
        fprintf(file_out, "del phy_tx_gain_soft\n");
        fprintf(file_out, "del phy_tx_freq\n");
        fprintf(file_out, "del phy_tx_lo_freq\n");
        fprintf(file_out, "del phy_tx_dsp_freq\n");
        fprintf(file_out, "del phy_tx_rate\n");    
        fprintf(file_out, "del phy_tx_mod_scheme\n");    
        fprintf(file_out, "del phy_tx_fec0\n");    
        fprintf(file_out, "del phy_tx_fec1\n");    
      }
      break;
    }

    // handle interferer tx logs
    case INT_TX:
    {
      struct timeval log_time;

      fprintf(file_out, "Int_tx_t       = list()\n");
      fprintf(file_out, "Int_tx_freq    = list()\n");

      float tx_freq;
      while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1,
                  file_in)){
        fread((float*)&tx_freq, sizeof(float), 1, file_in);
        fprintf(file_out, "Int_tx_t.append(%li + 1e-6*%li)\n",
                log_time.tv_sec, log_time.tv_usec);
        fprintf(file_out, "Int_tx_freq.append(%f)\n",  tx_freq);
        i++;
      }

      // If appending to the multiFile,
      // then put the data into the next elements of the multi array
      if (totalNumReps>1)
      {
        // Check that the multi array exists. Create it if it doesn't.
        fprintf(file_out,
                "if 'Int_tx_t_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    Int_tx_t_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'Int_tx_freq_all_repetitions' in locals():\n\n");
        fprintf(file_out, "    pass\n\n");
        fprintf(file_out, "else:\n\n");
        fprintf(file_out,
                "    Int_tx_freq_all_repetitions = [None]*%u\n\n",
                totalNumReps);

        // Place data in multiarray
        fprintf(file_out,
                "Int_tx_t_all_repetitions[%u] = Int_tx_t\n",
                repNumber-1);
        fprintf(file_out,
                "Int_tx_freq_all_repetitions[%u] = Int_tx_freq\n\n",
                repNumber-1);

        // Delete redundant data
        fprintf(file_out, "del Int_tx_t\n");
        fprintf(file_out, "del Int_tx_freq\n\n");
      }
      break;
    }

    // handle NET rx data logs
    case NET_RX:
    {
      struct timeval log_time;

      fprintf(file_out, "net_rx_t        = list()\n");
      fprintf(file_out, "net_rx_bytes    = list()\n");

      int bytes;
      while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1,
                  file_in)){
        fread((int*)&bytes, sizeof(int), 1, file_in);
        fprintf(file_out, "net_rx_t.append(%li + 1e-6*%li)\n",
                log_time.tv_sec, log_time.tv_usec);
        fprintf(file_out, "net_rx_bytes.append(%i)\n\n", bytes);
        i++;
      }

      // If appending to the multiFile, then put the data into the next elements of the multi array
      if (totalNumReps>1)
      {
        // Check that the multi array exists. Create it if it doesn't.
        fprintf(file_out,
                "if 'net_rx_t_all_repetitions' in locals():\n");
        fprintf(file_out, "    pass\n");
        fprintf(file_out, "else:\n");
        fprintf(file_out,
                "    net_rx_t_all_repetitions = [None]*%u\n",
                totalNumReps);
        fprintf(file_out,
                "if 'net_rx_bytes_all_repetitions' in locals():\n\n");
        fprintf(file_out, "    pass\n\n");
        fprintf(file_out, "else:\n\n");
        fprintf(file_out,
                "    net_rx_bytes_all_repetitions = [None]*%u\n\n",
                totalNumReps);

        // Place data in multiarray
        fprintf(file_out,
                "net_rx_t_all_repetitions[%u] = net_rx_t\n",
                repNumber-1);
        fprintf(file_out,
                "net_rx_bytes_all_repetitions[%u] = net_rx_bytes\n\n",
                repNumber-1);

        // Delete redundant data
        fprintf(file_out, "del net_rx_t\n");
        fprintf(file_out, "del net_rx_bytes\n\n");
      }
      break;

    }
  }

  fclose(file_in);
  fclose(file_out);
}
