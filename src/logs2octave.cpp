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
        printf(" -N : Total number of repetitions for this scenario (default: 1)");
        printf(" -n : The repetition number for this scenario (required if -N is given)");

}

int main(int argc, char ** argv){
    
    char log_file[100]; 
    char output_file[100];
    char multi_file[100];
    //char node_prefix[50];  
    //node_prefix[0] = '\0';

    enum log_t {
        RXMETRICS = 0,
        TXPARAMS,
        INTPARAMS,
        CRPARAMS
    }; 

    // default log type is receive metrics
    log_t log_type = RXMETRICS;
    
    strcpy(log_file, "logs/bin/");
    strcpy(output_file, "logs/octave/");
    strcpy(multi_file, "logs/octave/");
    
    unsigned int totalNumReps = 1;
    unsigned int repNumber = 1;

    

    // Option flags
    int l_opt = 0;
    int n_opt = 0;
    
    int d;
    //while((d = getopt(argc, argv, "hl:rtic")) != EOF){
    //while((d = getopt(argc, argv, "hl:p:rticN:n:")) != EOF){  
    while((d = getopt(argc, argv, "hl:rticN:n:")) != EOF){  
        switch(d){
        case 'h': help_post_process_logs();                 return 0;
        case 'l': 
        {
            strcat(log_file, optarg); 
            strcat(log_file, ".log");
            strcat(output_file, optarg);
            strcat(output_file, ".m");  
            strcat(multi_file, optarg);
            char *ptr_ = strrchr(multi_file, (int) '_');
            if (ptr_)
                multi_file[ ptr_ - multi_file] = '\0';
            strcat(multi_file, ".m");
            l_opt = 1;      
            break;
        }
	    //case 'o': strcpy(output_file, optarg); o_opt = 1;	   break;
	    //case 'p': strcpy(node_prefix, optarg);
		    //strcat(node_prefix, "_");           	    break;
        case 'r':                    		            break;
        case 't': log_type = TXPARAMS;                  break;
        case 'i': log_type = INTPARAMS;                 break;
        case 'c': log_type = CRPARAMS;                  break;
        case 'N': totalNumReps = atoi(optarg);          break;
        case 'n': repNumber = atoi(optarg);             
                    n_opt = 1;
                    break;
        }
    }

    // Check that log file and output file names were given
    if (!l_opt)
    {
        printf("Please give -l option.\n\n");
        help_post_process_logs();
        return 1;
    }

    if (totalNumReps>1 && !n_opt)
    {
        printf("-n option is required whenever -N option is given");
        help_post_process_logs();
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
    
    // handle ECR rx log

    if(log_type == RXMETRICS){

        fprintf(file_out, "t = [];\n");
        // metrics
        fprintf(file_out, "ECR_rx_Control_valid = [];\n");
        fprintf(file_out, "ECR_rx_Payload_valid = [];\n");
        fprintf(file_out, "ECR_rx_EVM = [];\n");
        fprintf(file_out, "ECR_rx_RSSI = [];\n");
        fprintf(file_out, "ECR_rx_CFO = [];\n");
        fprintf(file_out, "ECR_rx_num_syms = [];\n");    
        fprintf(file_out, "ECR_rx_mod_scheme = [];\n");
        fprintf(file_out, "ECR_rx_BPS = [];\n");
        fprintf(file_out, "ECR_rx_fec0 = [];\n");
        fprintf(file_out, "ECR_rx_fec1 = [];\n");
        // parameters
        fprintf(file_out, "ECR_rx_numSubcarriers = [];\n");
        fprintf(file_out, "ECR_rx_cp_len = [];\n");
        fprintf(file_out, "ECR_rx_taper_len = [];\n");
        fprintf(file_out, "ECR_rx_gain_uhd = [];\n");
        fprintf(file_out, "ECR_rx_freq = [];\n");
        fprintf(file_out, "ECR_rx_rate = [];\n");    

        while(fread((char*)&metrics, sizeof(struct ExtensibleCognitiveRadio::metric_s), 1, file_in)){
            fread((char*)&rx_params, sizeof(struct ExtensibleCognitiveRadio::rx_parameter_s), 1, file_in);
            fprintf(file_out, "t(%i) = %li + %f;\n", i, metrics.time_spec.get_full_secs(), metrics.time_spec.get_frac_secs());
            // metrics
            fprintf(file_out, "ECR_rx_Control_valid(%i) = %i;\n", i, metrics.control_valid);
            fprintf(file_out, "ECR_rx_Payload_valid(%i) = %i;\n", i, metrics.payload_valid);
            fprintf(file_out, "ECR_rx_EVM(%i) = %f;\n", i, metrics.stats.evm);
            fprintf(file_out, "ECR_rx_RSSI(%i) = %f;\n", i, metrics.stats.rssi);
            fprintf(file_out, "ECR_rx_CFO(%i) = %f;\n", i, metrics.stats.cfo);
            fprintf(file_out, "ECR_rx_num_syms(%i) = %i;\n", i, metrics.stats.num_framesyms);    
            fprintf(file_out, "ECR_rx_mod_scheme(%i) = %i;\n", i, metrics.stats.mod_scheme);
            fprintf(file_out, "ECR_rx_BPS(%i) = %i;\n", i, metrics.stats.mod_bps);
            fprintf(file_out, "ECR_rx_fec0(%i) = %i;\n", i, metrics.stats.fec0);
            fprintf(file_out, "ECR_rx_fec1(%i) = %i;\n\n", i, metrics.stats.fec1);
            // parameters
            fprintf(file_out, "ECR_rx_numSubcarriers(%i) = %u;\n", i, rx_params.numSubcarriers);
            fprintf(file_out, "ECR_rx_cp_len(%i) = %u;\n", i, rx_params.cp_len);
            fprintf(file_out, "ECR_rx_taper_len(%i) = %u;\n", i, rx_params.taper_len);
            fprintf(file_out, "ECR_rx_gain_uhd(%i) = %f;\n", i, rx_params.rx_gain_uhd);
            fprintf(file_out, "ECR_rx_freq(%i) = %f;\n", i, rx_params.rx_freq - rx_params.rx_dsp_freq);
            fprintf(file_out, "ECR_rx_rate(%i) = %f;\n", i, rx_params.rx_rate);    
            i++;
        }
        // If appending to the multiFile, then put the data into the next elements of the cell arrays
        if (totalNumReps>1)
        {
            fprintf(file_out, "t_all{%i} = t;\n", repNumber);
            // metrics
            fprintf(file_out, "ECR_rx_Control_valid_all{%i}     = ECR_rx_Control_valid;\n", repNumber);
            fprintf(file_out, "ECR_rx_Payload_valid_all{%i}     = ECR_rx_Payload_valid;\n", repNumber);
            fprintf(file_out, "ECR_rx_EVM_all{%i}   = ECR_rx_EVM;\n", repNumber);
            fprintf(file_out, "ECR_rx_RSSI_all{%i}  = ECR_rx_RSSI;\n", repNumber);
            fprintf(file_out, "ECR_rx_CFO_all{%i}   = ECR_rx_CFO;\n", repNumber);
            fprintf(file_out, "ECR_rx_num_syms_all{%i}  = ECR_rx_num_syms;\n", repNumber);
            fprintf(file_out, "ECR_rx_mod_scheme_all{%i}    = ECR_rx_mod_scheme;\n", repNumber);
            fprintf(file_out, "ECR_rx_BPS_all{%i}   = ECR_rx_BPS;\n", repNumber);
            fprintf(file_out, "ECR_rx_fec0_all{%i}  = ECR_rx_fec0;\n", repNumber);
            fprintf(file_out, "ECR_rx_fec1_all{%i}  = ECR_rx_fec1;\n", repNumber);
            // parameters
            fprintf(file_out, "ECR_rx_numSubcarriers_all{%i}    = ECR_rx_numSubcarriers;\n", repNumber);
            fprintf(file_out, "ECR_rx_cp_len_all{%i}    = ECR_rx_cp_len;\n", repNumber);
            fprintf(file_out, "ECR_rx_taper_len_all{%i}     = ECR_rx_taper_len;\n", repNumber);
            fprintf(file_out, "ECR_rx_gain_uhd_all{%i}  = ECR_rx_gain_uhd;\n", repNumber);
            fprintf(file_out, "ECR_rx_freq_all{%i}  = ECR_rx_freq;\n", repNumber);
            fprintf(file_out, "ECR_rx_rate_all{%i}  = ECR_rx_rate;\n", repNumber);

            // Delete redundant data
            // metrics
            fprintf(file_out, "clear ECR_rx_Control_valid;\n");
            fprintf(file_out, "clear ECR_rx_Payload_valid;\n");
            fprintf(file_out, "clear ECR_rx_EVM;\n");
            fprintf(file_out, "clear ECR_rx_RSSI;\n");
            fprintf(file_out, "clear ECR_rx_CFO;\n");
            fprintf(file_out, "clear ECR_rx_num_syms;\n");    
            fprintf(file_out, "clear ECR_rx_mod_scheme;\n");
            fprintf(file_out, "clear ECR_rx_BPS;\n");
            fprintf(file_out, "clear ECR_rx_fec0;\n");
            fprintf(file_out, "clear ECR_rx_fec1;\n\n");
            // parameters
            fprintf(file_out, "clear ECR_rx_numSubcarriers;\n");
            fprintf(file_out, "clear ECR_rx_cp_len;\n");
            fprintf(file_out, "clear ECR_rx_taper_len;\n");
            fprintf(file_out, "clear ECR_rx_gain_uhd;\n");
            fprintf(file_out, "clear ECR_rx_freq;\n");
            fprintf(file_out, "clear ECR_rx_rate;\n");    
        }
    }
    
    // handle ECR tx log
    if(log_type == TXPARAMS){
        struct timeval log_time;;

        fprintf(file_out, "ECR_tx_t = [];\n");
        fprintf(file_out, "ECR_tx_numSubcarriers = [];\n");
        fprintf(file_out, "ECR_tx_cp_len = [];\n");
        fprintf(file_out, "ECR_tx_taper_len = [];\n");
        fprintf(file_out, "ECR_tx_gain_uhd = [];\n");
        fprintf(file_out, "ECR_tx_gain_soft = [];\n");
        fprintf(file_out, "ECR_tx_freq = [];\n");
        fprintf(file_out, "ECR_tx_rate = [];\n");    

        while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1, file_in)){
            fread((char*)&tx_params, sizeof(struct ExtensibleCognitiveRadio::tx_parameter_s), 1, file_in);
            fprintf(file_out, "ECR_tx_t(%i) = %li + 1e-6*%li;\n", i, log_time.tv_sec, log_time.tv_usec);
            fprintf(file_out, "ECR_tx_numSubcarriers(%i) = %u;\n", i, tx_params.numSubcarriers);
            fprintf(file_out, "ECR_tx_cp_len(%i) = %u;\n", i, tx_params.cp_len);
            fprintf(file_out, "ECR_tx_taper_len(%i) = %u;\n", i, tx_params.taper_len);
            fprintf(file_out, "ECR_tx_gain_uhd(%i) = %f;\n", i, tx_params.tx_gain_uhd);
            fprintf(file_out, "ECR_tx_gain_soft(%i) = %f;\n", i, tx_params.tx_gain_soft);
            fprintf(file_out, "ECR_tx_freq(%i) = %f;\n", i, tx_params.tx_freq + tx_params.tx_dsp_freq);
            fprintf(file_out, "ECR_tx_rate(%i) = %f;\n", i, tx_params.tx_rate);    
            i++;
        }

        // If appending to the multiFile, then put the data into the next elements of the cell arrays
        if (totalNumReps>1)
        {
            fprintf(file_out, "ECR_tx_t_all{%i} = ECR_tx_t;\n", repNumber);
            fprintf(file_out, "ECR_tx_numSubcarriers_all{%i} = ECR_tx_numSubcarriers;\n", repNumber);
            fprintf(file_out, "ECR_tx_cp_len_all{%i} = ECR_tx_cp_len;\n", repNumber);
            fprintf(file_out, "ECR_tx_taper_len_all{%i} = ECR_tx_taper_len;\n", repNumber);
            fprintf(file_out, "ECR_tx_gain_uhd_all{%i} = ECR_tx_gain_uhd;\n", repNumber);
            fprintf(file_out, "ECR_tx_gain_soft_all{%i} = ECR_tx_gain_soft;\n", repNumber);
            fprintf(file_out, "ECR_tx_freq_all{%i} = ECR_tx_freq;\n", repNumber);
            fprintf(file_out, "ECR_tx_rate_all{%i} = ECR_tx_rate;\n", repNumber);

            // Delete redundant data
            fprintf(file_out, "clear ECR_tx_t;\n");
            fprintf(file_out, "clear ECR_tx_numSubcarriers;\n");
            fprintf(file_out, "clear ECR_tx_cp_len;\n");
            fprintf(file_out, "clear ECR_tx_taper_len;\n");
            fprintf(file_out, "clear ECR_tx_gain_uhd;\n");
            fprintf(file_out, "clear ECR_tx_gain_soft;\n");
            fprintf(file_out, "clear ECR_tx_freq;\n");
            fprintf(file_out, "clear ECR_tx_rate;\n");    
 
        }
    }

    // handle interferer tx logs
    if(log_type == INTPARAMS){
        struct timeval log_time;

        fprintf(file_out, "Int_tx_t = [];\n");
        fprintf(file_out, "Int_tx_freq = [];\n");

        float tx_freq;
        while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1, file_in)){
            fread((float*)&tx_freq, sizeof(float), 1, file_in);
            fprintf(file_out, "Int_tx_t(%i) = %li + 1e-6*%li;\n",  i, log_time.tv_sec, log_time.tv_usec);
            fprintf(file_out, "Int_tx_freq(%i) = %f;\n\n",  i, tx_freq);
            i++;
        }

        // If appending to the multiFile, then put the data into the next elements of the cell arrays
        if (totalNumReps>1)
        {
            fprintf(file_out, "Int_tx_t_all{%i} = Int_tx_t;\n", repNumber);
            fprintf(file_out, "Int_tx_freq_all{%i} = Int_tx_freq;\n\n", repNumber);

            // Delete redundant data
            fprintf(file_out, "clear Int_tx_t;\n");
            fprintf(file_out, "clear Int_tx_freq;\n\n");
        }

    }
    
    // handle CRTS rx data logs
    if(log_type == CRPARAMS){
        struct timeval log_time;

        fprintf(file_out, "CRTS_rx_t = [];\n");
        fprintf(file_out, "CRTS_rx_bytes = [];\n");

        int bytes;
        while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1, file_in)){
            fread((int*)&bytes, sizeof(int), 1, file_in);
            fprintf(file_out, "CRTS_rx_t(%i) = %li + 1e-6*%li;\n",  i, log_time.tv_sec, log_time.tv_usec);
            fprintf(file_out, "CRTS_rx_bytes(%i) = %i;\n\n",  i, bytes);
            i++;
        }    

        // If appending to the multiFile, then put the data into the next elements of the cell arrays
        if (totalNumReps>1)
        {
            fprintf(file_out, "CRTS_rx_t_all{%i} = CRTS_rx_t;\n", repNumber);
            fprintf(file_out, "CRTS_rx_bytes_all{%i} = CRTS_rx_bytes;\n\n", repNumber);

            // Delete redundant data
            fprintf(file_out, "clear CRTS_rx_t;\n");
            fprintf(file_out, "clear CRTS_rx_bytes;\n\n");
        }

    }

    fclose(file_in);
    fclose(file_out);
}
