#include <string>
#include <string.h>
#include <vector>
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <liquid/liquid.h>
#include "read_configs.hpp"
#include "node_parameters.hpp"

int read_scenario_master_file(char scenario_list[30][60])
{
	config_t cfg; // Returns all parameters in this structure
	char current_scenario[30];
	const char *tmpS;
	int num_scenarios = 1;
	int tmpI; // Stores the value of Integer Parameters from Config file
	
	config_init(&cfg);
	
	// Read the file. If there is an error, report it and exit.
	if (!config_read_file(&cfg, "master_scenario_file.cfg")){
		printf("Error reading master scenario file on line %i\n", config_error_line(&cfg));
		exit(1);
	}
	
	// Read the parameter group
	if (config_lookup_int(&cfg, "NumberofScenarios", &tmpI))
		num_scenarios = (int)tmpI;
		
	for (int i=0; i< num_scenarios; i++){
		sprintf(current_scenario, "scenario_%d", i+1);
		if (config_lookup_string(&cfg, current_scenario, &tmpS))
			strcpy(&scenario_list[i][0], tmpS);
	}
	config_destroy(&cfg);
	return num_scenarios;
} // End readScMasterFile()

struct scenario_parameters read_scenario_parameters(char * scenario_file)
{
	// configuration variable
	config_t cfg;
	config_init(&cfg);

	// string pointing to scenario file
	char scenario[100];
	strcpy(scenario, "scenarios/");
	strcat(scenario, scenario_file);

	// Read the file. If there is an error, report it and exit.
	if (!config_read_file(&cfg, scenario)){
		printf("Error reading %s on line %i\n", scenario, config_error_line(&cfg));
        printf("%s\n", config_error_text(&cfg));
		config_destroy(&cfg);
		exit(1);
	}

	// Read scenario parameters
	struct scenario_parameters sp;
	int tmpI;
	double tmpD;

	config_lookup_int(&cfg, "num_nodes", &tmpI);
	sp.num_nodes = tmpI;
	config_lookup_float(&cfg, "run_time", &tmpD);
	sp.run_time = (float) tmpD;
	config_destroy(&cfg);

	return sp;
} // End readScConfigFile()

struct node_parameters read_node_parameters(int node, char *scenario_file){
	// string pointing to scenario file
	char scenario[100];
	strcpy(scenario, "scenarios/");
	strcat(scenario, scenario_file);

	config_t cfg;
	config_init(&cfg);

	// Read the file. If there is an error, report it and exit.
	if (!config_read_file(&cfg, scenario))
	{
		printf("Error reading config file on line %i\n", config_error_line(&cfg));
		config_destroy(&cfg);
		exit(1);
	}
	
	int tmpI;
	double tmpD;
	const char *tmpS;
	
	// scenario info struct for node
	struct node_parameters np = {};
	char nodestr[100];
	std::string node_num;
	std::stringstream out;
	out << node;
	node_num = out.str();
	

	// lookup specific node
	strcpy(nodestr, "node");
	strcat(nodestr, node_num.c_str());
	config_setting_t *node_config = config_lookup(&cfg, nodestr);

	//printf("Read node %i config\n", node);

	// read CORNET IP address for the node
	if (config_setting_lookup_string(node_config, "TARGET_IP", &tmpS))
		strcpy(np.TARGET_IP, tmpS);

	// read CORNET IP address for the node
	if (config_setting_lookup_string(node_config, "CORNET_IP", &tmpS))
		strcpy(np.CORNET_IP, tmpS);

	// read type of node
	if (config_setting_lookup_string(node_config, "type", &tmpS)){
		//printf("\nNode type: %s\n", tmpS);
		if (strcmp(tmpS, "CR") == 0) 
        {
            np.type = CR;
            np.cr_type = ecr;
            //If node is a CR, lookup whether is uses the ECR or python
            if(config_setting_lookup_string(node_config, "cr_type", &tmpS))
            {
                if(strcmp(tmpS, "python") == 0)
                    np.cr_type = python;
            }
        }
		else if (strcmp(tmpS, "interferer") == 0) np.type = interferer;
	}

	// read all possible node settings
	if (config_setting_lookup_string(node_config, "CRTS_IP", &tmpS))
		strcpy(np.CRTS_IP, tmpS);

	if (config_setting_lookup_string(node_config, "CE", &tmpS))
		strcpy(np.CE, tmpS);

	if (config_setting_lookup_string(node_config, "traffic", &tmpS)){
		if(!strcmp(tmpS, "stream")) np.traffic = stream;
		else if(!strcmp(tmpS, "burst")) np.traffic = burst;
	}
	
	if (config_setting_lookup_int(node_config, "print_metrics", &tmpI))
		np.print_metrics = (int)tmpI;
	
	if (config_setting_lookup_int(node_config, "log_rx_metrics", &tmpI))
		np.log_rx_metrics = (int)tmpI;

	if (config_setting_lookup_int(node_config, "log_tx_parameters", &tmpI))
		np.log_tx_parameters = (int)tmpI;
	
	if (config_setting_lookup_string(node_config, "rx_log_file", &tmpS))
		strcpy(np.rx_log_file, tmpS);

	if (config_setting_lookup_string(node_config, "tx_log_file", &tmpS))
		strcpy(np.tx_log_file, tmpS);

	if (config_setting_lookup_float(node_config, "ce_timeout_ms", &tmpD))
		np.ce_timeout_ms = tmpD;

	if (config_setting_lookup_string(node_config, "duplex", &tmpS)){
		if(!strcmp(tmpS, "FDD")) np.duplex = FDD;
		else if(!strcmp(tmpS, "TDD")) np.duplex = TDD;
		else if(!strcmp(tmpS, "HD")) np.duplex = HD;
	}

	if (config_setting_lookup_float(node_config, "tx_freq", &tmpD))
		np.tx_freq = tmpD;

	if (config_setting_lookup_float(node_config, "rx_freq", &tmpD))
		np.rx_freq = tmpD;

	if (config_setting_lookup_float(node_config, "tx_rate", &tmpD))
		np.tx_rate = tmpD;

	if (config_setting_lookup_float(node_config, "rx_rate", &tmpD))
		np.rx_rate = tmpD;

	if (config_setting_lookup_float(node_config, "tx_gain_soft", &tmpD))
		np.tx_gain_soft = tmpD;

	if (config_setting_lookup_float(node_config, "tx_gain", &tmpD))
		np.tx_gain = tmpD;

	if (config_setting_lookup_float(node_config, "rx_gain", &tmpD))
		np.rx_gain = tmpD;

	if (config_setting_lookup_string(node_config, "tx_modulation", &tmpS)){

        // In case modulation scheme isn't found, set to unkown
        np.tx_modulation = LIQUID_MODEM_UNKNOWN;;

        // Iterate through every liquid modulation scheme 
        // and if the string matches, then assign that scheme.
        // See liquid soruce: src/modem/src/modem_utilities.c
        // for definition of modulation_types
        for (int k = 0; k<LIQUID_MODEM_NUM_SCHEMES; k++){
            if(!strcmp(tmpS, modulation_types[k].name))
                np.tx_modulation = modulation_types[k].scheme;
        }
	}

	if (config_setting_lookup_string(node_config, "tx_crc", &tmpS)){

        // In case CRC isn't found, set to unkown
        np.tx_crc = LIQUID_CRC_UNKNOWN;

        // Iterate through every liquid CRC
        // and if the string matches, then assign that CRC.
        // See liquid soruce: src/fec/src/crc.c
        // for definition of crc_scheme_str
        for (int k = 0; k<LIQUID_CRC_NUM_SCHEMES; k++){
            if(!strcmp(tmpS, crc_scheme_str[k][0]))
                np.tx_crc = k;
        }
	}

	if (config_setting_lookup_string(node_config, "tx_fec0", &tmpS)){

        // In case FEC isn't found, set to unkown
        np.tx_fec0 = LIQUID_FEC_UNKNOWN;

        // Iterate through every liquid FEC
        // and if the string matches, then assign that FEC.
        // See liquid soruce: src/fec/src/fec.c
        // for definition of fec_scheme_str
        for (int k = 0; k<LIQUID_FEC_NUM_SCHEMES; k++){
            if(!strcmp(tmpS, fec_scheme_str[k][0]))
                np.tx_fec0 = k;
        }
	}

	if (config_setting_lookup_string(node_config, "tx_fec1", &tmpS)){

        // In case FEC isn't found, set to unkown
        np.tx_fec1 = LIQUID_FEC_UNKNOWN;

        // Iterate through every liquid FEC
        // and if the string matches, then assign that FEC.
        // See liquid soruce: src/fec/src/fec.c
        // for definition of fec_scheme_str
        for (int k = 0; k<LIQUID_FEC_NUM_SCHEMES; k++){
            if(!strcmp(tmpS, fec_scheme_str[k][0]))
                np.tx_fec1 = k;
        }
	}
	if (config_setting_lookup_float(node_config, "tx_delay_us", &tmpD))
		np.tx_delay_us = tmpD;

	if (config_setting_lookup_string(node_config, "interference_type", &tmpS)){
		if(!strcmp(tmpS, "CW"))
			np.interference_type = CW;
		if(!strcmp(tmpS, "AWGN")) 
			np.interference_type = AWGN;
		if(!strcmp(tmpS, "GMSK"))
			np.interference_type = GMSK;
		if(!strcmp(tmpS, "RRC")) 
			np.interference_type = RRC;
		if(!strcmp(tmpS, "OFDM")) 
			np.interference_type = OFDM;
	}
	
	if (config_setting_lookup_float(node_config, "period_duration", &tmpD))
		np.period_duration = tmpD;

	if (config_setting_lookup_float(node_config, "duty_cycle", &tmpD))
		np.duty_cycle = tmpD;

        // ======================================================
        // process frequency hopping parameters
        // ======================================================
	if (config_setting_lookup_string(node_config, 
                                         "tx_freq_hop_type", 
                                         &tmpS))
          {
	  if(!strcmp(tmpS, "NONE"))
	    np.tx_freq_hop_type = NONE;
	  if(!strcmp(tmpS, "ALTERNATING"))
	    np.tx_freq_hop_type = ALTERNATING;
	  if(!strcmp(tmpS, "SWEEP")) 
	    np.tx_freq_hop_type = SWEEP;
	  if(!strcmp(tmpS, "RANDOM"))
	    np.tx_freq_hop_type = RANDOM;
	  }
	
	if (config_setting_lookup_float(node_config, 
                                        "tx_freq_hop_min", 
                                        &tmpD))
		np.tx_freq_hop_min = tmpD;

	if (config_setting_lookup_float(node_config, 
                                        "tx_freq_hop_max", 
                                        &tmpD))
		np.tx_freq_hop_max = tmpD;

	if (config_setting_lookup_float(node_config, 
                                        "tx_freq_hop_dwell_time", 
                                        &tmpD))
		np.tx_freq_hop_dwell_time = tmpD;

	if (config_setting_lookup_float(node_config, 
                                        "tx_freq_hop_increment", 
                                        &tmpD))
		np.tx_freq_hop_increment = tmpD;


        // ======================================================
        // process GMSK interference parameters
        // ======================================================
	if (config_setting_lookup_float(node_config, 
                                        "gmsk_header_length", 
                                        &tmpD))
		np.gmsk_header_length = tmpD;

	if (config_setting_lookup_float(node_config, 
                                        "gmsk_payload_length", 
                                        &tmpD))
		np.gmsk_payload_length = tmpD;

	if (config_setting_lookup_float(node_config, 
                                        "gmsk_bandwidth", 
                                        &tmpD))
		np.gmsk_bandwidth = tmpD;



	return np;
}

void print_node_parameters(struct node_parameters * np)
  {
  printf("\n");
  printf("--------------------------------------------------------------\n");
  printf("-                    node parameters                         -\n");
  printf("--------------------------------------------------------------\n");
  printf("General:\n");
  char node_type[15] = "UNKNOWN";
  if(np->type == CR) 
    {
    strcpy(node_type, "CR");
    }
  else if(np->type == interferer) 
    {
    strcpy(node_type, "Interferer");
    }

  printf("	Node type:                         %-s\n", node_type);
  if(np->type == CR)
  {
      char cr_type[15] = "ecr";
      if(np->cr_type == python)
          strcpy(cr_type, "python");
      printf("	CR type:                           %-s\n", cr_type);
  }
  printf("	CORNET IP:                         %-s\n", np->CORNET_IP);
  if(np->type != interferer)
    {
    printf("	CRTS IP:                           %-s\n", np->CRTS_IP);
    printf("	Target IP:                         %-s\n", np->TARGET_IP);
    printf("	Cognitive Engine:                  %-s\n", np->CE);
    }
  if(np->type == CR)
    {
    printf("	Traffic type:                      %-i\n", np->traffic);
    }
  printf("	Rx log file:                       %-s\n", np->rx_log_file);
  printf("	Tx log file:                       %-s\n", np->tx_log_file);
  printf("	CE timeout:                        %-.2f\n", np->ce_timeout_ms);
  printf("RF:\n");
  if(np->type != interferer)
    {
    char duplex[4] = "FDD";
    switch(np->duplex)
      {
      case (FDD): strcpy(duplex, "FDD"); break;
      case (TDD): strcpy(duplex, "TDD"); break;
      case (HD): strcpy(duplex, "HD"); break;
      }
    printf("	Duplex scheme:                     %-s\n", duplex);
    }

  printf("	Transmit frequency:                %-.2e\n", np->tx_freq);
  if(np->type != interferer)
    {
    printf("	Receive frequency:                 %-.2e\n", np->rx_freq);
    }
  printf("	Transmit rate:                     %-.2e\n", np->tx_rate);
  if(np->type != interferer)
    {
    printf("	Receive rate:                      %-.2e\n", np->rx_rate);
    }
  if(np->type != interferer)
    {
    printf("	Transmit soft gain:                %-.2f\n", np->tx_gain_soft);
    }
  printf("	Transmit gain:                     %-.2f\n", np->tx_gain);
  if(np->type != interferer)
    {
    printf("	Receive gain:                      %-.2f\n", np->rx_gain);
    }

  if(np->type == interferer)
    {
    char interference_type[5] = "NONE";
    char tx_freq_hop_type[6] = "NONE"; 
    switch(np->interference_type)
      {
      case (CW): strcpy(interference_type, "CW"); break;
      case (AWGN): strcpy(interference_type, "AWGN"); break;
      case (GMSK): strcpy(interference_type, "GMSK"); break;
      case (RRC): strcpy(interference_type, "RRC"); break;
      case (OFDM): strcpy(interference_type, "OFDM"); break;
      }
    switch(np->tx_freq_hop_type)
      {
      case (NONE): strcpy(tx_freq_hop_type, "NONE"); break;
      case (ALTERNATING): strcpy(tx_freq_hop_type, "ALTERNATING"); break;
      case (SWEEP): strcpy(tx_freq_hop_type, "SWEEP"); break;
      case (RANDOM): strcpy(tx_freq_hop_type, "RANDOM"); break;
      }
    printf("	Interference type:                 %-s\n", interference_type);
    printf("	Interference period_duration:	   %-.2f\n", np->period_duration);
    printf("	Interference duty cycle:           %-.2f\n", np->duty_cycle);
    printf("\n"); 
    printf("	tx freq hop type:                  %-s\n", tx_freq_hop_type);
    printf("	tx freq hop min:                   %-.2e\n", np->tx_freq_hop_min);
    printf("	tx freq hop max:                   %-.2e\n", np->tx_freq_hop_max);
    printf("	tx freq hop dwell time:            %-.2f\n", np->tx_freq_hop_dwell_time);
    printf("	tx freq hop increment:             %-.2e\n", np->tx_freq_hop_increment);

    printf("\n"); 
    printf("	GMSK header length:                %-.2d\n", np->gmsk_header_length);
    printf("	GMSK payload length:               %-.2d\n", np->gmsk_payload_length);
    printf("	GMSK bandwidth:                    %-.2e\n", np->gmsk_bandwidth);
    }
  printf("--------------------------------------------------------------\n");
  }










