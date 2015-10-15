#include <stdio.h>
#include "ECR.hpp"

void help_logs2python() {
        printf("logs2python -- Create Python .py file containing log data.\n");
        printf("               The new file can then be imported into a Python script to process the data.\n");
        printf(" -h : Help.\n");
        printf(" -l : Name of log file to process (required).\n");
        printf(" -o : Name of output file (required).\n");
        printf(" -p : Optional prefix for Python variable names.\n");
        printf(" -r : Log file contains cognitive radio receive metrics.\n");
        printf(" -t : Log file contains cognitive radio transmit parameters.\n");
        printf(" -i : Log file contains interferer transmit parameters.\n");
}

int main(int argc, char ** argv){
    
    char log_file[50]; 
    char output_file[50];
    char node_prefix[50];
    strcpy(node_prefix, "");
    
    enum log_t {
        RXMETRICS = 0,
        TXPARAMS,
        INTPARAMS
    };

    // default log type is receive metrics
    log_t log_type = RXMETRICS;

    // Option flags
    int l_opt = 0;
    int o_opt = 0;

    int d;
    while((d = getopt(argc, argv, "hl:o:p:rti")) != EOF){
        switch(d){
        case 'h': help_logs2python();                       return 0;
        case 'l': strcpy(log_file, optarg); l_opt = 1;      break;
        case 'o': strcpy(output_file, optarg); o_opt = 1;   break;
        case 'p': strcpy(node_prefix, optarg); 
                    strcat(node_prefix, "_");               break;
        case 'r':                                           break;
        case 't': log_type = TXPARAMS;                      break;
        case 'i': log_type = INTPARAMS;                     break;
        }
    }

    // Check that log file and output file names were given
    if (!l_opt)
    {
        printf("Please give -l option.\n\n");
        help_logs2python();
        return 1;
    }
    if (!o_opt)
    {
        printf("Please give -o option.\n\n");
        help_logs2python();
        return 1;
    }

    // Check that output file name has .py extension. If not add one.
    char fileExtension[4];
    strcpy(fileExtension, ".py");
    if ( strcmp(output_file+(strlen(output_file)-strlen(fileExtension)), fileExtension ) )
        strcat(output_file, fileExtension);

    printf("Log file name: %s\n", log_file);
    printf("Output file name: %s\n", output_file);

    FILE * file_in = fopen(log_file, "rb");
    FILE * file_out = fopen(output_file, "w");

    struct ExtensibleCognitiveRadio::metric_s metrics = {};
    struct ExtensibleCognitiveRadio::tx_parameter_s tx_params = {};
    int i = 1;
    
    if(log_type == RXMETRICS){
        fprintf(file_out,   "%st                      = list()\n", node_prefix);
        fprintf(file_out,   "%sECR_rx_Control_valid    = list()\n", node_prefix);
        fprintf(file_out,   "%sECR_rx_Payload_valid   = list()\n", node_prefix);
        fprintf(file_out,   "%sECR_rx_EVM             = list()\n", node_prefix);
        fprintf(file_out,   "%sECR_rx_RSSI            = list()\n", node_prefix);
        fprintf(file_out,   "%sECR_rx_CFO             = list()\n", node_prefix);
        fprintf(file_out,   "%sECR_rx_num_syms        = list()\n", node_prefix);    
        fprintf(file_out,   "%sECR_rx_mod_scheme      = list()\n", node_prefix);
        fprintf(file_out,   "%sECR_rx_BPS             = list()\n", node_prefix);
        fprintf(file_out,   "%sECR_rx_fec0            = list()\n", node_prefix);
        fprintf(file_out,   "%sECR_rx_fec1            = list()\n", node_prefix);

        while(fread((char*)&metrics, sizeof(struct ExtensibleCognitiveRadio::metric_s), 1, file_in)){
            fprintf(file_out, "%st.append(%li + %f)\n",                 node_prefix,    metrics.time_spec.get_full_secs(), 
                        metrics.time_spec.get_frac_secs());
            fprintf(file_out, "%sECR_rx_Control_valid.append(%i)\n",     node_prefix,    metrics.control_valid);
            fprintf(file_out, "%sECR_rx_Payload_valid.append(%i)\n",    node_prefix,    metrics.payload_valid);
            fprintf(file_out, "%sECR_rx_EVM.append(%f)\n",              node_prefix,    metrics.stats.evm);
            fprintf(file_out, "%sECR_rx_RSSI.append(%f)\n",             node_prefix,    metrics.stats.rssi);
            fprintf(file_out, "%sECR_rx_CFO.append(%f)\n",              node_prefix,    metrics.stats.cfo);
            fprintf(file_out, "%sECR_rx_num_syms.append(%i)\n",         node_prefix,    metrics.stats.num_framesyms);    
            fprintf(file_out, "%sECR_rx_mod_scheme.append(%i)\n",       node_prefix,    metrics.stats.mod_scheme);
            fprintf(file_out, "%sECR_rx_BPS.append(%i)\n",              node_prefix,    metrics.stats.mod_bps);
            fprintf(file_out, "%sECR_rx_fec0.append(%i)\n",             node_prefix,    metrics.stats.fec0);
            fprintf(file_out, "%sECR_rx_fec1.append(%i)\n",             node_prefix,    metrics.stats.fec1);
            i++;
        }
    }
    else if(log_type == TXPARAMS){
        fprintf(file_out, "%sECR_tx_t         = list()\n", node_prefix);
        fprintf(file_out, "%sECR_tx_numSubcarriers         = list()\n", node_prefix);
        fprintf(file_out, "%sECR_tx_cp_len    = list()\n", node_prefix);
        fprintf(file_out, "%sECR_tx_taper_len = list()\n", node_prefix);
        fprintf(file_out, "%sECR_tx_gain_uhd  = list()\n", node_prefix);
        fprintf(file_out, "%sECR_tx_gain_soft = list()\n", node_prefix);
        fprintf(file_out, "%sECR_tx_freq      = list()\n", node_prefix);
        fprintf(file_out, "%sECR_tx_rate      = list()\n", node_prefix);    

        struct timeval log_time;
        while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1, file_in)){
            fread((char*)&tx_params, sizeof(struct ExtensibleCognitiveRadio::tx_parameter_s), 1, file_in);
            fprintf(file_out, "%sECR_tx_t.append(%li + 1e-6*%li)\n", node_prefix, log_time.tv_sec, log_time.tv_usec);
            fprintf(file_out, "%sECR_tx_numSubcarriers.append(%u)\n",              node_prefix, tx_params.numSubcarriers);
            fprintf(file_out, "%sECR_tx_cp_len.append(%u)\n",         node_prefix, tx_params.cp_len);
            fprintf(file_out, "%sECR_tx_taper_len.append(%u)\n",      node_prefix, tx_params.taper_len);
            fprintf(file_out, "%sECR_tx_gain_uhd.append(%f)\n",       node_prefix, tx_params.tx_gain_uhd);
            fprintf(file_out, "%sECR_tx_gain_soft.append(%f)\n",      node_prefix, tx_params.tx_gain_soft);
            fprintf(file_out, "%sECR_tx_freq.append(%f)\n",           node_prefix, tx_params.tx_freq);
            fprintf(file_out, "%sECR_tx_rate.append(%f)\n",           node_prefix, tx_params.tx_rate);    
            i++;
        }
    }

    if(log_type == INTPARAMS){
        fprintf(file_out, "%sInt_tx_t       = list()\n", node_prefix);
        fprintf(file_out, "%sInt_tx_freq    = list()\n", node_prefix);
        struct timeval log_time;
        float tx_freq;
        while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1, file_in)){
            fread((float*)&tx_freq, sizeof(float), 1, file_in);
            fprintf(file_out, "%sInt_tx_t.append(%li + 1e-6*%li)\n", node_prefix, log_time.tv_sec, log_time.tv_usec);
            fprintf(file_out, "%sInt_tx_freq.append(%f)\n",  node_prefix, tx_freq);
            i++;
        }
    }

    fclose(file_in);
    fclose(file_out);
}
