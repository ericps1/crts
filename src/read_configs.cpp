#include<string.h>
#include<libconfig.h>
#include<stdio.h>
#include<stdlib.h>
#include<sstream>
#include"node_parameters.hpp"

int readScMasterFile(char scenario_list[30][60], int verbose)
{
	config_t cfg; // Returns all parameters in this structure
	config_setting_t *setting;
	const char *str; // Stores the value of the String Parameters in Config file
	int tmpI; // Stores the value of Integer Parameters from Config file
	char current_sc[30];
	int no_of_scenarios = 1;
	int i;
	char tmpS[30];
	//Initialization
	config_init(&cfg);
	// Read the file. If there is an error, report it and exit.
	if (!config_read_file(&cfg, "master_scenario_file.cfg")){
		printf("Error reading master scenario file\n");
		exit(1);
	}
	// Read the parameter group
	setting = config_lookup(&cfg, "params");
	if (setting != NULL)
	{
		if (config_setting_lookup_int(setting, "NumberofScenarios", &tmpI))
		{
			no_of_scenarios = tmpI;
			if (verbose)
				printf("Number of Scenarios: %d\n", tmpI);
		}
		for (i = 1; i <= no_of_scenarios; i++)
		{
			strcpy(current_sc, "scenario_");
			sprintf(tmpS, "%d", i);
			strcat(current_sc, tmpS);
			if (config_setting_lookup_string(setting, current_sc, &str))
			{
				strcpy(*((scenario_list)+i - 1), str);
			}
			if (verbose)
				printf("Scenario File: %s\n", *((scenario_list)+i - 1));
		}
	}
	config_destroy(&cfg);
	return no_of_scenarios;
} // End readScMasterFile()

int read_num_nodes(char *scenario_file)
{
	// configuration variable
	config_t cfg;
	config_init(&cfg);

	// string pointing to scenario file
	//char scenario[100];
	//strcpy(scenario, "scenarios/");
	//strcat(scenario, scenario_file);

	// Read the file. If there is an error, report it and exit.
	if (!config_read_file(&cfg, "scenarios/scenario.cfg"))
	{
		printf("Error reading number of nodes\n");
		config_destroy(&cfg);
		exit(1);
	}

	// Read number of nodes
	int num_nodes;
	config_lookup_int(&cfg, "num_nodes", &num_nodes);
	config_destroy(&cfg);

	return num_nodes;
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
	
	//int tmpI;
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
		if (strcmp(tmpS,"BS")) np.type = BS;
		else if (strcmp(tmpS, "UE")) np.type = UE;
		else if (strcmp(tmpS, "interferer")) np.type = interferer;
		}

	// read all possible node settings
	if (config_setting_lookup_string(node_config, "CRTS_IP", &tmpS))
		strcpy(np.CRTS_IP, tmpS);

	if (config_setting_lookup_string(node_config, "CE", &tmpS))
		strcpy(np.CE, tmpS);

	if (config_setting_lookup_string(node_config, "traffic", &tmpS)){
		if(strcmp(tmpS, "stream")) np.traffic = stream;
		else if(strcmp(tmpS, "burst")) np.traffic = burst;
	}
	if (config_setting_lookup_float(node_config, "freq_tx", &tmpD))
		np.freq_tx = tmpD;

	if (config_setting_lookup_float(node_config, "freq_rx", &tmpD))
		np.freq_rx = tmpD;

	if (config_setting_lookup_float(node_config, "tx_rate", &tmpD))
		np.tx_rate = tmpD;

	if (config_setting_lookup_float(node_config, "rx_rate", &tmpD))
		np.rx_rate = tmpD;

	if (config_setting_lookup_float(node_config, "gain_tx", &tmpD))
		np.gain_tx = tmpD;

	if (config_setting_lookup_float(node_config, "gain_rx", &tmpD))
		np.gain_rx = tmpD;

	return np;
}

void print_node_parameters(struct node_parameters * np){
	printf("\n");
	printf("------------------------------------------------\n");
	printf("-               node parameters                -\n");
	printf("------------------------------------------------\n");
	printf("General:\n");
	printf("	CORNET IP:                 %s\n", np->CORNET_IP);
	printf("	CRTS IP:                   %s\n", np->CRTS_IP);
	printf("	Node type:                 %i\n", np->type);
	if(np->type != interferer)
	printf("	Cognitive Engine:          %s\n", np->CE);
	if(np->type == UE);
	printf("	Traffic type:              %i\n", np->traffic);
	printf("RF:\n");
	printf("	Transmit frequency:        %.2e\n", np->freq_tx);
	printf("	Receive frequency:         %.2e\n", np->freq_rx);
	printf("	Transmit rate:             %.2e\n", np->tx_rate);
	printf("	Receive rate:              %.2e\n", np->rx_rate);
	printf("	Transmit soft gain:        %.2e\n", np->gain_tx_soft);
	printf("	Transmit gain:             %.2e\n", np->gain_tx);
	printf("	Receive gain:              %.2e\n", np->gain_rx);
	if(np->type == interferer){
	printf("	Interference type:         %i\n", np->int_type);
	printf("	Interference duty cycle:   %.2f\n", np->duty_cycle);
	}
}










