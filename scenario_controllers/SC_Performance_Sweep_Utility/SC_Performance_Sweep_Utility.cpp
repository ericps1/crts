#include <stdio.h>
#include <liquid/liquid.h>
#include <libconfig.h>
#include "SC_Performance_Sweep_Utility.hpp"

// constructor
SC_Performance_Sweep_Utility::SC_Performance_Sweep_Utility(int argc, char **argv) {

  // initialize parameters
  dwell_time_s = DEFAULT_DWELL_TIME_S;
  settle_time_s = DEFAULT_SETTLE_TIME_S;
  node_1_feedback_received = false;
  node_2_feedback_received = false;

  // set up sweep management
  sweep_mode = SWEEP_MODE_NESTED;
  data_ind = 0;
  strcpy(sweep_cfg_file, "scenario_controllers/SC_Performance_Sweep_Utility/");
  strcpy(log_file_name, "logs/csv/");

  // interpret command line options
  int o;
  bool cfg_file_given = false;
  bool log_file_given = false;
  while ((o = getopt(argc, argv, "d:s:c:l:")) != EOF) {
    switch (o) {
      case 'd': // dwell time argument
        dwell_time_s = atoi(optarg);
        break;
      case 's': // step time argument
        settle_time_s = atoi(optarg);
        break;
      case 'c': // config file argument
        strcat(sweep_cfg_file,optarg);
        cfg_file_given = true;
        break;
      case 'l': // log file argument
        strcat(log_file_name,optarg);
        log_file_given = true;
        break; 
    }
  }

  if (cfg_file_given == false)
    strcat(sweep_cfg_file, "default_sweep.cfg");
  else
    strcat(sweep_cfg_file, ".cfg");

  if (log_file_given == false)
    strcat(log_file_name, "performance_sweep_log.csv");
  else
    strcat(log_file_name, ".csv");

  printf("Reading sweep config file: %s\n", sweep_cfg_file); 
  read_sweep_cfg();

  // setup storage for rx statistics
  if (sweep_mode == SWEEP_MODE_NESTED) {
    num_data_points = 2;
    for (int i=0; i<num_sweep_params; i++)
      num_data_points *= num_vals[i];
  } else {
    num_data_points = 2*num_vals[0];
  }
  rx_stats.resize(num_data_points); 
 
}

// destructor
SC_Performance_Sweep_Utility::~SC_Performance_Sweep_Utility() {

  log = fopen(log_file_name, "w");
  for (int i=0; i<num_sweep_params; i++) {
    fprintf(log, "%s,", crts_param_str[params[i]]);
  }
  fprintf(log, "node,frames received,valid frames,EVM (dB),RSSI (dB),PER,BER,"
    "Throughput (b/s),uhd overflows\n");
  char sweep_vals_str[1024]; 
  int ind;
  
  // determine the maximum step period for the swept parameters
  int max_param_period = 2; // initialized at 2 for 2 nodes
  for (int i=0; i<num_sweep_params; i++)
    max_param_period *= num_vals[i];

  // iterate through all data points, sweeping parameter values are determined
  // based on the nested loop structure
  for (int i=0; i<num_data_points; i++){
    int param_period = max_param_period; // the first parameter has the greatest period
    memset(sweep_vals_str,0,sizeof(sweep_vals_str));
    for (int j=0; j<num_sweep_params; j++) {
      param_period /= num_vals[j];
      ind = (sweep_mode == SWEEP_MODE_NESTED) ? (i/param_period)%num_vals[j] : i/2;
      switch (params[j]) {
        case (CRTS_TX_FREQ):
        case (CRTS_TX_RATE):
        case (CRTS_TX_GAIN):
        case (CRTS_RX_FREQ):
        case (CRTS_RX_RATE):
        case (CRTS_RX_GAIN):
        case (CRTS_NET_THROUGHPUT):
        case (CRTS_TX_DUTY_CYCLE):
        case (CRTS_TX_PERIOD):
        case (CRTS_TX_FREQ_MIN):
        case (CRTS_TX_FREQ_MAX):
        case (CRTS_TX_FREQ_DWELL_TIME):
        case (CRTS_TX_FREQ_RES):
          sprintf(sweep_vals_str, "%s%.3e", sweep_vals_str,
            (*(std::vector<double>*)vals[j])[ind]);
          break;
        case (CRTS_TX_STATE):
        case (CRTS_RX_STATE):
          sprintf(sweep_vals_str, "%s%i", sweep_vals_str,
            (*(std::vector<int>*)vals[j])[ind]);
          break;
        case (CRTS_TX_MOD):
          sprintf(sweep_vals_str, "%s%s", sweep_vals_str,
            modulation_types[(*(std::vector<int>*)vals[j])[ind]].name);
          break;
        case (CRTS_TX_FEC0):
        case (CRTS_TX_FEC1):
          sprintf(sweep_vals_str, "%s%s", sweep_vals_str,
            fec_scheme_str[(*(std::vector<int>*)vals[j])[ind]][0]);
          break;
        case (CRTS_NET_TRAFFIC_TYPE):
          printf("%s: place holder\n", crts_param_str[params[j]]);//, 
          //fec_scheme_str[(*(std::vector<int>*)vals[i])[indices[i]]][1]);
          break;
        case (CRTS_TX_FREQ_BEHAVIOR):
          break; 
      }
      sprintf(sweep_vals_str, "%s,", sweep_vals_str); 
    }
    fprintf(log, "%s", sweep_vals_str);
    fprintf(log, "%i,", (i%2)+1);
    fprintf(log, "%i,", rx_stats[i].frames_received);
    fprintf(log, "%i,", rx_stats[i].valid_frames);
    fprintf(log, "%.3f,", rx_stats[i].evm_dB);
    fprintf(log, "%.3f,", rx_stats[i].rssi_dB);
    fprintf(log, "%.3f,", rx_stats[i].per);
    fprintf(log, "%.3e,", rx_stats[i].ber);
    fprintf(log, "%.3e,", rx_stats[i].throughput);
    fprintf(log, "%i\n", rx_stats[i].uhd_overflows);
  }
  fprintf(log, "\n\n");
  fclose(log);  
  

  for (int i=0; i<num_sweep_params; i++) {
    switch (crts_get_param_type(params[i])){
      case (CRTS_PARAM_DOUBLE):
        delete(static_cast<std::vector<double>*>(vals[i]));
        break;
      case (CRTS_PARAM_INT):
        delete(static_cast<std::vector<int>*>(vals[i]));
        break; 
    }
  }  
}

// setup feedback enables for each node
void SC_Performance_Sweep_Utility::initialize_node_fb() {

  int fb_enables = CRTS_RX_STATS_FB_EN; 
  set_node_parameter(1, CRTS_FB_EN, (void*) &fb_enables);
  set_node_parameter(2, CRTS_FB_EN, (void*) &fb_enables);

  double rx_stats_period = dwell_time_s;
  set_node_parameter(1, CRTS_RX_STATS, (void*) &rx_stats_period); 
  set_node_parameter(1, CRTS_RX_STATS_FB, (void*) &rx_stats_period);
  set_node_parameter(2, CRTS_RX_STATS, (void*) &rx_stats_period); 
  set_node_parameter(2, CRTS_RX_STATS_FB, (void*) &rx_stats_period);

  set_params(0); // initializes all sweep parameters on nodes
  print_sweep_point_summary();    
}

// execute function
void SC_Performance_Sweep_Utility::execute() { 
  
  if ((sc_event == FEEDBACK) && (fb.fb_type == CRTS_RX_STATS)) {
    
    if (data_ind < num_data_points)
      rx_stats[data_ind+fb.node-1] = *(struct ExtensibleCognitiveRadio::rx_statistics*) fb.arg;
    else
      printf("WARNING: The sweep has finished. The following data will not "
             "be included in the results\n\n");

    printf("Node %i has sent updated receive statistics:\n", fb.node);
    printf("  Number of frames received: %i\n", 
      (*(struct ExtensibleCognitiveRadio::rx_statistics*)fb.arg).frames_received);
    printf("  Number of frames valid: %i\n", 
      (*(struct ExtensibleCognitiveRadio::rx_statistics*)fb.arg).valid_frames);
    printf("  Average EVM:               %.3f\n", 
      (*(struct ExtensibleCognitiveRadio::rx_statistics*)fb.arg).evm_dB);
    printf("  Average RSSI:              %.3f\n", 
      (*(struct ExtensibleCognitiveRadio::rx_statistics*)fb.arg).rssi_dB);
    printf("  Average PER:               %.3f\n", 
      (*(struct ExtensibleCognitiveRadio::rx_statistics*)fb.arg).per);
    printf("  Average BER:               %.3f\n", 
      (*(struct ExtensibleCognitiveRadio::rx_statistics*)fb.arg).ber);
    printf("  Average throughput:        %.3e\n", 
      (*(struct ExtensibleCognitiveRadio::rx_statistics*)fb.arg).throughput); 
    printf("  UHD overflows:             %i\n\n",
      (*(struct ExtensibleCognitiveRadio::rx_statistics*)fb.arg).uhd_overflows);

    if (fb.node == 1) node_1_feedback_received = true;
    if (fb.node == 2) node_2_feedback_received = true;
    
    if (node_1_feedback_received && node_2_feedback_received) {
      data_ind += 2;
      node_1_feedback_received = false;
      node_2_feedback_received = false;
      update_sweep_params();
      print_sweep_point_summary();
    }
  }
}

void SC_Performance_Sweep_Utility::print_sweep_point_summary() {

  printf("Current sweep parameters:\n");
  for (int i=0; i<num_sweep_params; i++) {
    switch (params[i]) {
      case (CRTS_TX_FREQ):
      case (CRTS_TX_RATE):
      case (CRTS_TX_GAIN):
      case (CRTS_RX_FREQ):
      case (CRTS_RX_RATE):
      case (CRTS_RX_GAIN):
      case (CRTS_NET_THROUGHPUT):
      case (CRTS_TX_DUTY_CYCLE):
      case (CRTS_TX_PERIOD):
      case (CRTS_TX_FREQ_MIN):
      case (CRTS_TX_FREQ_MAX):
      case (CRTS_TX_FREQ_DWELL_TIME):
      case (CRTS_TX_FREQ_RES):
        printf("%s: %.3e\n", crts_param_str[params[i]], 
          (*(std::vector<double>*)vals[i])[indices[i]]);
        break;
      case (CRTS_TX_STATE):
      case (CRTS_RX_STATE):
        printf("%s: %i\n", crts_param_str[params[i]], 
          (*(std::vector<int>*)vals[i])[indices[i]]);
        break;
      case (CRTS_TX_MOD):
        printf("%s: %s\n", crts_param_str[params[i]], 
          modulation_types[(*(std::vector<int>*)vals[i])[indices[i]]].name);
        break;
      case (CRTS_TX_FEC0):
      case (CRTS_TX_FEC1):
        printf("%s: %s\n", crts_param_str[params[i]], 
          fec_scheme_str[(*(std::vector<int>*)vals[i])[indices[i]]][1]);
        break;
      case (CRTS_NET_TRAFFIC_TYPE):
        printf("%s: place holder\n", crts_param_str[params[i]]);//, 
          //fec_scheme_str[(*(std::vector<int>*)vals[i])[indices[i]]][1]);
        break;
      case (CRTS_TX_FREQ_BEHAVIOR):
        break;
      default:
        printf("Unrecognized parameter type\n");
        exit(EXIT_FAILURE);
    } 
  }
  printf("\n");
}

void SC_Performance_Sweep_Utility::update_sweep_params() {

  // if nested sweep is used we need to update the inner most
  // loop first, then propagate up if it rolls over
  if (sweep_mode == SWEEP_MODE_NESTED) {
    for (int i=num_sweep_params-1; i>=0; i--) {
      indices[i]++;
      if (indices[i] == num_vals[i]) {
        for (int j=num_sweep_params-1; j>=i; j--)
          indices[j] = 0;
      } else {
        set_params(i);
        break;
      }
    }
  } else {
    for (int i=0; i<num_sweep_params; i++) {
      indices[i]++;
      if (indices[i] == num_vals[i])
        indices[i] = 0;
      set_params(i);
    }
  }

}

void SC_Performance_Sweep_Utility::set_params(int param_ind) {

  for (int i=param_ind; i<num_sweep_params; i++) {
    switch(crts_get_param_type(params[i])) {
      case (CRTS_PARAM_INT):
        set_node_parameter(1,params[i],(void*) &((*(std::vector<int>*)vals[i])[indices[i]]));
        set_node_parameter(2,params[i],(void*) &((*(std::vector<int>*)vals[i])[indices[i]]));  
        break;
      case (CRTS_PARAM_DOUBLE):
        set_node_parameter(1,params[i],(void*) &((*(std::vector<double>*)vals[i])[indices[i]]));
        set_node_parameter(2,params[i],(void*) &((*(std::vector<double>*)vals[i])[indices[i]]));  
        if (params[i] == CRTS_TX_RATE) {
          set_node_parameter(1,CRTS_RX_RATE,(void*) &((*(std::vector<double>*)vals[i])[indices[i]]));
          set_node_parameter(2,CRTS_RX_RATE,(void*) &((*(std::vector<double>*)vals[i])[indices[i]])); 
        }
        break;
      default:
        printf("Unknown crts parameter type\n");
        exit(EXIT_FAILURE);
    }
  }

  sleep(settle_time_s);
  set_node_parameter(1,CRTS_RX_RESET,NULL);
  set_node_parameter(2,CRTS_RX_RESET,NULL);
}

void SC_Performance_Sweep_Utility::alloc_vals(int i) {

  switch (crts_get_param_type(params[i])){
    case (CRTS_PARAM_DOUBLE):
      vals[i] = (void*) new std::vector<double>;
      (*(std::vector<double>*)vals[i]).resize(num_vals[i]); 
      break;
    case (CRTS_PARAM_INT):
      vals[i] = (void*) new std::vector<int>; 
      (*(std::vector<int>*)vals[i]).resize(num_vals[i]); 
      break;
    default:
      printf("Parameter type not recognized\n");
      exit(EXIT_FAILURE);
  }    
}

void SC_Performance_Sweep_Utility::read_sweep_cfg() {

  config_t cfg;
  config_setting_t *param_cfg;
  char cfg_str[30];
  const char *tmpS;
  int tmpI;
  double tmpD;

  printf("Reading sweep config\n");

  config_init(&cfg);

  if (!config_read_file(&cfg, sweep_cfg_file)) {
    printf("Error reading sweep config file (%s) on line %i\n",
           sweep_cfg_file, config_error_line(&cfg));
    exit(EXIT_FAILURE);
  }

  sprintf(cfg_str, "sweep_mode");
  if (config_lookup_string(&cfg, cfg_str, &tmpS)) {
    if (!strcmp(tmpS,"nested"))
      sweep_mode = SWEEP_MODE_NESTED;
    else if (!strcmp(tmpS,"step"))
      sweep_mode = SWEEP_MODE_STEP;
  } else {
    printf("Number of sweep parameters not specified\n");
    exit(EXIT_FAILURE);
  }

  sprintf(cfg_str, "num_sweep_params");
  if (config_lookup_int(&cfg, cfg_str, &tmpI)) {
    num_sweep_params = tmpI;
    params.resize(num_sweep_params,0);
    num_vals.resize(num_sweep_params,0);
    vals.resize(num_sweep_params,0);
    indices.resize(num_sweep_params,0);
  } else {
    printf("Number of sweep parameters not specified\n");
    exit(EXIT_FAILURE);
  }

  for (int i=0; i<num_sweep_params; i++) {
    sprintf(cfg_str, "param_%i", i+1);
    param_cfg = config_lookup(&cfg, cfg_str);
    sprintf(cfg_str, "param_type");
    if (config_setting_lookup_string(param_cfg, cfg_str, &tmpS)) {
      params[i] = crts_get_str2param(tmpS);
      
      // either the sweep parameter provides specific values or a range and step size
      sprintf(cfg_str, "num_vals");
      if (config_setting_lookup_int(param_cfg, cfg_str, &num_vals[i])){
        
        if ((sweep_mode == SWEEP_MODE_STEP) && (num_vals[i] != num_vals[0])) {
          printf("In step mode the number of values specified for each parameter should "
                 "be the same.");
          exit(EXIT_FAILURE);
        }

        alloc_vals(i);
        for (int j=0; j<num_vals[i]; j++) {
          sprintf(cfg_str, "val_%i", j+1); 
          switch (params[i]) {
            case (CRTS_TX_FREQ):
            case (CRTS_TX_RATE):
            case (CRTS_TX_GAIN):
            case (CRTS_RX_FREQ):
            case (CRTS_RX_RATE):
            case (CRTS_RX_GAIN):
            case (CRTS_NET_THROUGHPUT):
            case (CRTS_TX_DUTY_CYCLE):
            case (CRTS_TX_PERIOD):
            case (CRTS_TX_FREQ_MIN):
            case (CRTS_TX_FREQ_MAX):
            case (CRTS_TX_FREQ_DWELL_TIME):
            case (CRTS_TX_FREQ_RES):
              if (config_setting_lookup_float(param_cfg, cfg_str, &tmpD)){
                (*(std::vector<double>*)vals[i])[j] = tmpD; 
              } else {
                printf("Double value %i for parameter %i not specified\n",j+1,i+1);
                exit(EXIT_FAILURE);
              }
              break;
            case (CRTS_TX_STATE):
            case (CRTS_RX_STATE):
              if (config_setting_lookup_int(param_cfg, cfg_str, &tmpI)){
                (*(std::vector<int>*)vals[i])[j] = tmpI; 
              } else {
                printf("Int value %i for parameter %i not specified\n",j+1,i+1);
                exit(EXIT_FAILURE);
              }
              break;
            default:
              if (config_setting_lookup_string(param_cfg, cfg_str, &tmpS)) {
                switch(params[i]) {
                  case (CRTS_TX_MOD):
                    (*(std::vector<int>*)vals[i])[j] = liquid_getopt_str2mod(tmpS);
                    break;
                  case (CRTS_TX_FEC0):
                  case (CRTS_TX_FEC1):
                    (*(std::vector<int>*)vals[i])[j] = liquid_getopt_str2fec(tmpS);
                    break;
                  case (CRTS_NET_TRAFFIC_TYPE):
                    (*(std::vector<int>*)vals[i])[j] = crts_get_str2net_traffic_type(tmpS);
                    break;
                  case (CRTS_TX_FREQ_BEHAVIOR):
                    (*(std::vector<int>*)vals[i])[j] = crts_get_str2tx_freq_behavior(tmpS);
                    break;
                  default:
                    printf("Unrecognized parameter type\n");
                    exit(EXIT_FAILURE);
                }
              } else {
                printf("Value %i for parameter %i not specified\n",j,i);
                exit(EXIT_FAILURE);
              }
          } // switch(params[i])
        } // for num_vals[i]
      } else { // num_vals was not specified, assume a range and step were provided
        double initial_val;
        double final_val;
        double step_val;
        sprintf(cfg_str, "initial_val");
        if (!config_setting_lookup_float(param_cfg, cfg_str, &initial_val)){
          printf("Neither initial_val or num_vals were specified %i\n",i);
          exit(EXIT_FAILURE);
        }
        sprintf(cfg_str, "final_val");
        if (!config_setting_lookup_float(param_cfg, cfg_str, &final_val)){
          printf("final_val was not specified %i\n",i);
          exit(EXIT_FAILURE);
        }
        sprintf(cfg_str, "step_val");
        if (!config_setting_lookup_float(param_cfg, cfg_str, &step_val)){
          printf("step val was not specified %i\n",i);
          exit(EXIT_FAILURE);
        }
      
        // translate the range and step size into values
        num_vals[i] = ((final_val-initial_val)/step_val)+1; 
        alloc_vals(i);
      
        for (int j=0; j<num_vals[i]; j++) {
          switch (crts_get_param_type(params[i])){
            case (CRTS_PARAM_DOUBLE):
              (*(std::vector<double>*)vals[i])[j] = initial_val + j*step_val;
              break;
            case (CRTS_PARAM_INT):
              (*(std::vector<int>*)vals[i])[j] = (int) (initial_val + j*step_val); 
              break;
            default:
              printf("Parameter type not recognized\n");
              exit(EXIT_FAILURE);
          }         
        }
      }
    } else {
      printf("Failed to find config setting for vals_%i\n", i+1);
      exit(EXIT_FAILURE);
    } 
  } // for num_sweep_params

}


