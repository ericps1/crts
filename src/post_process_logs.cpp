#include <stdio.h>
#include "ECR.hpp"

void help_post_process_logs() {
        printf("post_process_logs -- Create Octave .m file to visualize logs.\n");
        printf(" -h : Help.\n");
        printf(" -l : Name of log file to process (required).\n");
        printf(" -o : Name of output file (required).\n");
		printf(" -r : Log file contains cognitive radio receive metrics.\n");
		printf(" -t : Log file contains cognitive radio transmit parameters.\n");
        printf(" -i : Log file contains interferer transmit parameters");
}

int main(int argc, char ** argv){
	
	char log_file[50]; 
	char output_file[50];
	
	// default log type is receive metrics
	int log_type = 0;

	// Option flags
    int l_opt = 0;
    int o_opt = 0;

	int d;
	while((d = getopt(argc, argv, "hl:o:rti")) != EOF){
		switch(d){
		case 'h': help_post_process_logs();                 return 0;
		case 'l': strcpy(log_file, optarg); l_opt = 1;      break;
		case 'o': strcpy(output_file, optarg); o_opt = 1;   break;
		case 'r':                                           break;
		case 't': log_type = 1;                             break;
		case 'i': log_type = 2;                             break;
		}
	}

    // Check that log file and output file names were given
    if (!l_opt)
    {
        printf("Please give -l option.\n\n");
        help_post_process_logs();
        return 1;
    }
    if (!o_opt)
    {
        printf("Please give -o option.\n\n");
        help_post_process_logs();
        return 1;
    }

	printf("Log file name: %s\n", log_file);
	printf("Output file name: %s\n", output_file);

	FILE * file_in = fopen(log_file, "rb");
	FILE * file_out = fopen(output_file, "w");

	struct metric_s metrics = {};
	struct tx_parameter_s tx_params = {};
	int i = 1;
	
	//fprintf(file_out, "clear all;\n");
	//fprintf(file_out, "close all;\n");

	if(log_type == 0){
	while(fread((char*)&metrics, sizeof(struct metric_s), 1, file_in)){
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

	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "stem(ECR_rx_t,ECR_rx_Header_valid);\n");
	fprintf(file_out, "title('Valid Headers');\n");	
	fprintf(file_out, "xlabel('Time (s)');\n");
	fprintf(file_out, "ylabel('Valid Header');\n");
	fprintf(file_out, "ylim([-1 2]);\n\n");

	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "stem(ECR_rx_t,ECR_rx_Payload_valid);\n");
	fprintf(file_out, "title(\"Valid Payloads\");\n");	
	fprintf(file_out, "xlabel(\"Time (s)\");\n");
	fprintf(file_out, "ylabel(\"Valid Payload\");\n");
	fprintf(file_out, "ylim([-1 2]);\n\n");

	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_rx_t,ECR_rx_EVM);\n");
	fprintf(file_out, "title(\"Error Vector Magnitide\");\n");	
	fprintf(file_out, "xlabel(\"Time (s)\");\n");
	fprintf(file_out, "ylabel(\"EVM (dB)\");\n\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_rx_t,ECR_rx_RSSI);\n");
	fprintf(file_out, "title(\"Received Signal Strength Indicator\");\n");	
	fprintf(file_out, "xlabel(\"Time (s)\");\n");
	fprintf(file_out, "ylabel(\"RSSI (dB)\");\n\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_rx_t,ECR_rx_CFO);\n");
	fprintf(file_out, "title(\"Carrier Frequency Offset\");\n");	
	fprintf(file_out, "xlabel(\"Time (s)\");\n");
	fprintf(file_out, "ylabel(\"CFO (f/fs)\");\n\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_rx_t,ECR_rx_num_syms);\n");
	fprintf(file_out, "title(\"Number of Frame Symbols\");\n");	
	fprintf(file_out, "xlabel(\"Time (s)\");\n");
	fprintf(file_out, "ylabel(\"Number of Frame Symbols\");\n\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_rx_t,ECR_rx_mod_scheme);\n");
	fprintf(file_out, "title(\"Modulation Scheme\");\n");	
	fprintf(file_out, "xlabel(\"Time (s)\");\n");
	fprintf(file_out, "ylabel(\"Modulation Scheme\");\n");
	fprintf(file_out, "ylim([25 30]);\n");
	fprintf(file_out, "labels = {'Unknown','PSK2','PSK4','PSK8','PSK16','PSK32','PSK64','PSK128','PSK256','DPSK2','DPSK4','DPSK8','DPSK16','DPSK32','DPSK64','DPSK128','DPSK256','ASK2','ASK4','ASK8','ASK16','ASK32','ASK64','ASK128','ASK256','QAM4', 'QAM16', 'QAM32', 'QAM64', 'QAM128', 'QAM256','APSK2','APSK4','APSK8','APSK16','APSK32','APSK64','APSK128','APSK256','BPSK','QPSK','OOK','SQAM32','SQAM128','V29','Optimal QAM16','Optimal QAM32','Optimal QAM64','Optimal QAM128','Optimal QAM256','VT Logo'};\n");
	fprintf(file_out, "set(gca, 'YTick', 0:52, 'YTickLabel', labels);\n\n");

	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_rx_t,ECR_rx_BPS);\n");
	fprintf(file_out, "title(\"Bits Per Symbol\");\n");	
	fprintf(file_out, "xlabel(\"Time (s)\");\n");
	fprintf(file_out, "ylabel(\"Bits Per Symbol\");\n");
	fprintf(file_out, "ylim([0, max(BPS)+1]);\n\n");

	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_rx_t,ECR_rx_fec0);\n");
	fprintf(file_out, "title(\"Inner Forward Error Correction\");\n");	
	fprintf(file_out, "xlabel(\"Time (s)\");\n");
	fprintf(file_out, "ylabel(\"FEC Scheme\");\n");
	fprintf(file_out, "labels = {'Unknown','None','Repeat (1/3)','Repeat(1/5)','Hamming (7/4)','Hamming (8/4)','Hamming (12/8)','Golay (24/12)','SEC-DED (22/16)','SEC-DED (22/16)','SEC-DED (39/32)','SEC-DED72/64)','Convultional (1/2,7)','Convolutional (1/2,9)','Convolutional (1/3,9)','Convolutional (1/6,15)','Convolutional (2/3,7)','Convolutional (3/4,7)','Convolutional (4/5,7)','Convolutional (5/6,7)','Convolutional(6/7,7)','Convolutional (7/8,7)','Convolutional (2/3,9)','Convolutional (3/4,9)','Convolutional (4/5,9)','Convolutional (5/6,9)','Convolutional(6/7,9)','Convolutional(7/8,9)','Reed-Solomon (8)'};");
	fprintf(file_out, "set(gca, 'YTick', 0:28, 'YTickLabel', labels);\n\n");

	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_rx_t,ECR_rx_fec1);\n");
	fprintf(file_out, "title(\"Outter Forward Error Correction\");\n");	
	fprintf(file_out, "xlabel(\"Time (s)\");\n");
	fprintf(file_out, "ylabel(\"FEC Scheme\");\n");
	fprintf(file_out, "labels = {'Unknown','None','Repeat (1/3)','Repeat(1/5)','Hamming (7/4)','Hamming (8/4)','Hamming (12/8)','Golay (24/12)','SEC-DED (22/16)','SEC-DED (22/16)','SEC-DED (39/32)','SEC-DED72/64)','Convultional (1/2,7)','Convolutional (1/2,9)','Convolutional (1/3,9)','Convolutional (1/6,15)','Convolutional (2/3,7)','Convolutional (3/4,7)','Convolutional (4/5,7)','Convolutional (5/6,7)','Convolutional(6/7,7)','Convolutional (7/8,7)','Convolutional (2/3,9)','Convolutional (3/4,9)','Convolutional (4/5,9)','Convolutional (5/6,9)','Convolutional(6/7,9)','Convolutional(7/8,9)','Reed-Solomon (8)'};");
	fprintf(file_out, "set(gca, 'YTick', 0:28, 'YTickLabel', labels);\n\n");
	
	}
	else if(log_type == 1){
	struct timeval log_time;
    while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1, file_in)){
		fread((char*)&tx_params, sizeof(struct tx_parameter_s), 1, file_in);
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

	fprintf(file_out, "\nECR_tx_t = ECR_tx_t - ECR_tx_t(1);\n\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_tx_t, ECR_tx_M);\n");
	fprintf(file_out, "title('Number of subcarriers');\n");	
	fprintf(file_out, "xlabel('Time (s)');\n");
	fprintf(file_out, "ylabel('Number of Subcarriers');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_tx_t, ECR_tx_cp_len);\n");
	fprintf(file_out, "title('Length of Cyclic Prefix');\n");	
	fprintf(file_out, "xlabel('Time (s)');\n");
	fprintf(file_out, "ylabel('Samples');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_tx_t, ECR_tx_taper_len);\n");
	fprintf(file_out, "title('Taper Length');\n");	
	fprintf(file_out, "xlabel('Time (s)');\n");
	fprintf(file_out, "ylabel('Samples');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_tx_t, ECR_tx_gain_uhd);\n");
	fprintf(file_out, "title('Tx USRP Gain');\n");	
	fprintf(file_out, "xlabel('Time (s)');\n");
	fprintf(file_out, "ylabel('Gain (dB)');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_tx_t, ECR_tx_gain_soft);\n");
	fprintf(file_out, "title('Tx Soft Gain');\n");	
	fprintf(file_out, "xlabel('Time (s)');\n");
	fprintf(file_out, "ylabel('Gain (dB)');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_tx_t, ECR_tx_freq);\n");
	fprintf(file_out, "title('Tx Center Frequency');\n");	
	fprintf(file_out, "xlabel('Time (s)');\n");
	fprintf(file_out, "ylabel('Frequency (Hz)');\n");
	fprintf(file_out, "ylim([2*min(ECR_tx_freq)-max(ECR_tx_freq), 2*max(ECR_tx_freq)-min(ECR_tx_freq)]);\n");

	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(ECR_tx_t, ECR_tx_rate);\n");
	fprintf(file_out, "title('Tx Rate');\n");	
	fprintf(file_out, "xlabel('Time (s)');\n");
	fprintf(file_out, "ylabel('Tx Rate (Hz)');\n");
	
	}

    if(log_type == 2){
	struct timeval log_time;
    float tx_freq;
	while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1, file_in)){
		fread((float*)&tx_freq, sizeof(float), 1, file_in);
		fprintf(file_out, "Int_tx_t(%i) = %li + 1e-6*%li;\n", i, log_time.tv_sec, log_time.tv_usec);
		fprintf(file_out, "Int_tx_freq(%i) = %f;\n\n", i, tx_freq);
		i++;
	}

	fprintf(file_out, "\nInt_tx_t = Int_tx_t - Int_tx_t(1);\n\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(Int_tx_t,Int_tx_freq);\n");
	fprintf(file_out, "title('Transmit Frequency');\n");	
	fprintf(file_out, "xlabel('Time (s)');\n");
	fprintf(file_out, "ylabel('Frequency (Hz)');\n");
	fprintf(file_out, "ylim([2*min(Int_tx_freq)-max(Int_tx_freq), 2*max(Int_tx_freq)-min(Int_tx_freq)]);\n");
	}

	fclose(file_in);
	fclose(file_out);
}
