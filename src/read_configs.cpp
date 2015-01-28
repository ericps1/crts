#include <string.h>
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
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
		printf("Error reading master scenario file\n");
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

struct scenario_parameters read_scenario_parameters(char *scenario_file)
{
	// configuration variable
	config_t cfg;
	config_init(&cfg);

	// string pointing to scenario file
	char scenario[100];
	strcpy(scenario, "scenarios/");
	strcat(scenario, scenario_file);

	// Read the file. If there is an error, report it and exit.
	if (!config_read_file(&cfg, scenario)){//"scenarios/scenario.cfg")){
		printf("Error reading number of nodes\n");
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
		printf("Error reading config file\n");
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
	if (config_setting_lookup_string(node_config, "CORNET_IP", &tmpS))
		strcpy(np.CORNET_IP ,tmpS);

	// read type of node
	if (config_setting_lookup_string(node_config, "type", &tmpS)){
		//printf("\nNode type: %s\n", tmpS);
		if (!strcmp(tmpS,"BS")) np.type = BS;
		else if (!strcmp(tmpS, "UE")) np.type = UE;
		else if (!strcmp(tmpS, "interferer")) np.type = interferer;
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
	
	if (config_setting_lookup_int(node_config, "log_metrics", &tmpI))
		np.log_metrics = (int)tmpI;
	
	if (config_setting_lookup_string(node_config, "log_file", &tmpS))
		strcpy(np.log_file, tmpS);

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

	if (config_setting_lookup_int(node_config, "int_type", &tmpI))
		np.int_type = (int)tmpI;
	
	if (config_setting_lookup_float(node_config, "period", &tmpD))
		np.period = tmpD;

	if (config_setting_lookup_float(node_config, "duty_cycle", &tmpD))
		np.duty_cycle = tmpD;

	return np;
}

void print_node_parameters(struct node_parameters * np){
	printf("\n");
	printf("------------------------------------------------\n");
	printf("-               node parameters                -\n");
	printf("------------------------------------------------\n");
	printf("General:\n");
	char node_type[15];
	if(np->type == UE) strcpy(node_type, "UE");
	else if(np->type == BS) strcpy(node_type, "BS");
	else if(np->type == interferer) strcpy(node_type, "Interferer");
	printf("	Node type:                 %-s\n", node_type);
	printf("	CORNET IP:                 %-s\n", np->CORNET_IP);
	if(np->type != interferer){
	printf("	CRTS IP:                   %-s\n", np->CRTS_IP);
	printf("	Cognitive Engine:          %-s\n", np->CE); }
	if(np->type == UE)
	printf("	Traffic type:              %-i\n", np->traffic);
	printf("	Log file:                  %-s\n", np->log_file);
	printf("RF:\n");
	printf("	Transmit frequency:        %-.2e\n", np->tx_freq);
	if(np->type != interferer)
	printf("	Receive frequency:         %-.2e\n", np->rx_freq);
	printf("	Transmit rate:             %-.2e\n", np->tx_rate);
	if(np->type != interferer)
	printf("	Receive rate:              %-.2e\n", np->rx_rate);
	if(np->type != interferer)
	printf("	Transmit soft gain:        %-.2e\n", np->tx_gain_soft);
	printf("	Transmit gain:             %-.2e\n", np->tx_gain);
	if(np->type != interferer)
	printf("	Receive gain:              %-.2e\n", np->rx_gain);
	if(np->type == interferer){
	char int_type[5];
	if(np->int_type == CW) strcpy(int_type, "CW");
	else if(np->int_type == RRC) strcpy(int_type, "RRC");
	printf("	Interference type:         %-s\n", int_type);
	printf("	Interference period:	   %-.2f\n", np->period);
	printf("	Interference duty cycle:   %-.2f\n", np->duty_cycle);
	}
	printf("------------------------------------------------\n");
}










