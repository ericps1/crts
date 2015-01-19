#include <stdio.h>
#include "CR.hpp"

int main(){
	
	char log_name[] = "/users/ericps1/crts/logs/log2.bin";
	FILE * file_in = fopen(log_name, "rb");

	char output_name[] = "/users/ericps1/crts/logs/log2.m";
	FILE * file_out = fopen(output_name, "a");

	struct metric_s metrics = {};
	//float evm;
	int i = 1;
	//while(file_in.read((char*)&metrics, sizeof(struct metric_s))){
	while(fread((char*)&metrics, sizeof(struct metric_s), 1, file_in)){
		printf("EVM: %e\n", metrics.stats.evm);
		fprintf(file_out, "Header_valid(%i) = %i\n", i, metrics.header_valid);
		fprintf(file_out, "Payload_valid(%i) = %i\n", i, metrics.payload_valid);
		fprintf(file_out, "EVM(%i) = %f\n", i, metrics.stats.evm);
		fprintf(file_out, "RSSI(%i) = %f\n", i, metrics.stats.rssi);
		fprintf(file_out, "CFO(%i) = %f\n", i, metrics.stats.cfo);
		fprintf(file_out, "num_syms(%i) = %i\n", i, metrics.stats.num_framesyms);	
		fprintf(file_out, "mod_scheme(%i) = %i\n", i, metrics.stats.mod_scheme);
		fprintf(file_out, "BPS(%i) = %i\n", i, metrics.stats.mod_bps);
		fprintf(file_out, "fec0(%i) = %i\n", i, metrics.stats.fec0);
		fprintf(file_out, "fec1(%i) = %i\n\n", i, metrics.stats.fec1);
		i++;
	}

	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(HEADER_valid);\n");
	fprintf(file_out, "title(Valid Headers);\n");	
	fprintf(file_out, "xlabel('Received Packet Number');\n");
	fprintf(file_out, "ylabel('Valid Header');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(Payload_valid);\n");
	fprintf(file_out, "title(Valid Payloads);\n");	
	fprintf(file_out, "xlabel('Received Packet Number');\n");
	fprintf(file_out, "ylabel('Valid Payload');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(EVM);\n");
	fprintf(file_out, "title(Error Vector Magnitide);\n");	
	fprintf(file_out, "xlabel('Received Packet Number');\n");
	fprintf(file_out, "ylabel('EVM (dB)');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(RSSI);\n");
	fprintf(file_out, "title(Received Signal Strength Indicator);\n");	
	fprintf(file_out, "xlabel('Received Packet Number');\n");
	fprintf(file_out, "ylabel('RSSI (dB)');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(CFO);\n");
	fprintf(file_out, "title(Carrier Frequency Offset);\n");	
	fprintf(file_out, "xlabel('Received Packet Number');\n");
	fprintf(file_out, "ylabel(CFO (f/fs));\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(num_syms);\n");
	fprintf(file_out, "title(Number of Frame Symbols);\n");	
	fprintf(file_out, "xlabel('Received Packet Number');\n");
	fprintf(file_out, "ylabel('Number of Frame Symbols');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(mod_scheme);\n");
	fprintf(file_out, "title(Modulation Scheme);\n");	
	fprintf(file_out, "xlabel('Received Packet Number');\n");
	fprintf(file_out, "ylabel('Modulation Scheme');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(BPS);\n");
	fprintf(file_out, "title(Bits Per Symbol);\n");	
	fprintf(file_out, "xlabel('Received Packet Number');\n");
	fprintf(file_out, "ylabel('Bits Per Symbol');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(fec0);\n");
	fprintf(file_out, "title(Inner Forward Error Correction);\n");	
	fprintf(file_out, "xlabel('Received Packet Number');\n");
	fprintf(file_out, "ylabel('FEC Scheme');\n");
	
	fprintf(file_out, "\nfigure;\n");
	fprintf(file_out, "plot(fec1);\n");
	fprintf(file_out, "title(Outter Forward Error Correction);\n");	
	fprintf(file_out, "xlabel('Received Packet Number');\n");
	fprintf(file_out, "ylabel('FEC Scheme');\n");
	
	fclose(file_in);
	fclose(file_out);
}
