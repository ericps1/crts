#include <stdio.h>
#include "ECR.hpp"

void help_post_process_logs() {
        printf("post_process_logs -- Create Octave .m file to visualize logs.\n");
        printf(" -h : Help.\n");
        printf(" -l : Name of log file to process (required).\n");
        printf(" -r : Log file contains cognitive radio receive metrics.\n");
        printf(" -t : Log file contains cognitive radio transmit parameters.\n");
        printf(" -i : Log file contains interferer transmit parameters.\n");
        printf(" -c : Log file contains CRTS rx data.\n");
}

int main(int argc, char ** argv){
    
    char log_file[100]; 
    char output_file[100];
    
    strcpy(log_file, "logs/bin/");
    strcpy(output_file, "logs/octave/");
    
    // default log type is receive metrics
    int log_type = 0;

    // Option flags
    int l_opt = 0;
    
    int d;
    while((d = getopt(argc, argv, "hl:rtic")) != EOF){
        switch(d){
        case 'h': help_post_process_logs();                 return 0;
        case 'l': 
            strcat(log_file, optarg); 
            strcat(log_file, ".log");
            strcat(output_file, optarg);
            strcat(output_file, ".m");
            l_opt = 1;      
            break;
        case 'r':                                           break;
        case 't': log_type = 1;                             break;
        case 'i': log_type = 2;                             break;
        case 'c': log_type = 3;                                break;
        }
    }

    // Check that log file and output file names were given
    if (!l_opt)
    {
        printf("Please give -l option.\n\n");
        help_post_process_logs();
        return 1;
    }
    
    printf("Log file name: %s\n", log_file);
    printf("Output file name: %s\n", output_file);

    FILE * file_in = fopen(log_file, "rb");
    FILE * file_out = fopen(output_file, "w");

    struct ExtensibleCognitiveRadio::metric_s metrics = {};
    struct ExtensibleCognitiveRadio::tx_parameter_s tx_params = {};
    int i = 1;
    
    // handle ECR rx log
    if(log_type == 0){
        while(fread((char*)&metrics, sizeof(struct ExtensibleCognitiveRadio::metric_s), 1, file_in)){
            fprintf(file_out, "t(%i) = %li + %f;\n", i, metrics.time_spec.get_full_secs(), metrics.time_spec.get_frac_secs());
            fprintf(file_out, "ECR_rx_Header_valid(%i) = %i;\n", i, metrics.header_valid);
            fprintf(file_out, "ECR_rx_Payload_valid(%i) = %i;\n", i, metrics.payload_valid);
            fprintf(file_out, "ECR_rx_EVM(%i) = %f;\n", i, metrics.stats.evm);
            fprintf(file_out, "ECR_rx_RSSI(%i) = %f;\n", i, metrics.stats.rssi);
            fprintf(file_out, "ECR_rx_CFO(%i) = %f;\n", i, metrics.stats.cfo);
            fprintf(file_out, "ECR_rx_num_syms(%i) = %i;\n", i, metrics.stats.num_framesyms);    
            fprintf(file_out, "ECR_rx_mod_scheme(%i) = %i;\n", i, metrics.stats.mod_scheme);
            fprintf(file_out, "ECR_rx_BPS(%i) = %i;\n", i, metrics.stats.mod_bps);
            fprintf(file_out, "ECR_rx_fec0(%i) = %i;\n", i, metrics.stats.fec0);
            fprintf(file_out, "ECR_rx_fec1(%i) = %i;\n\n", i, metrics.stats.fec1);
            i++;
        }
    }
    
    // handle ECR tx log
    if(log_type == 1){
        struct timeval log_time;
        while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1, file_in)){
            fread((char*)&tx_params, sizeof(struct ExtensibleCognitiveRadio::tx_parameter_s), 1, file_in);
            fprintf(file_out, "ECR_tx_t(%i) = %li + 1e-6*%li;\n", i, log_time.tv_sec, log_time.tv_usec);
            fprintf(file_out, "ECR_tx_M(%i) = %u;\n", i, tx_params.M);
            fprintf(file_out, "ECR_tx_cp_len(%i) = %u;\n", i, tx_params.cp_len);
            fprintf(file_out, "ECR_tx_taper_len(%i) = %u;\n", i, tx_params.taper_len);
            fprintf(file_out, "ECR_tx_gain_uhd(%i) = %f;\n", i, tx_params.tx_gain_uhd);
            fprintf(file_out, "ECR_tx_gain_soft(%i) = %f;\n", i, tx_params.tx_gain_soft);
            fprintf(file_out, "ECR_tx_freq(%i) = %f;\n", i, tx_params.tx_freq);
            fprintf(file_out, "ECR_tx_rate(%i) = %f;\n", i, tx_params.tx_rate);    
            i++;
        }
    }

    // handle interferer tx logs
    if(log_type == 2){
        struct timeval log_time;
        float tx_freq;
        while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1, file_in)){
            fread((float*)&tx_freq, sizeof(float), 1, file_in);
            fprintf(file_out, "Int_tx_t(%i) = %li + 1e-6*%li;\n", i, log_time.tv_sec, log_time.tv_usec);
            fprintf(file_out, "Int_tx_freq(%i) = %f;\n\n", i, tx_freq);
            i++;
        }
    }
    
    // handle CRTS rx data logs
    if(log_type == 3){
        struct timeval log_time;
        int bytes;
        while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1, file_in)){
            fread((int*)&bytes, sizeof(int), 1, file_in);
            fprintf(file_out, "CRTS_rx_t(%i) = %li + 1e-6*%li;\n", i, log_time.tv_sec, log_time.tv_usec);
            fprintf(file_out, "CRTS_rx_bytes(%i) = %i;\n\n", i, bytes);
            i++;
        }    
    }

    fclose(file_in);
    fclose(file_out);
}
