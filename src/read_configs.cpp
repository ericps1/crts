struct node_settings_s{
	enum type{
		AP;
		UE;
		interferer;
	}

	char *CORNET_IP;
	char *CRTS_IP;
	char *AP;
	char *CE;
	char *Interference;

	bool PHY_metrics;
	bool MAC_metrics;
	bool NET_metrics;

	int traffic;

	float freq_tx;
	float freq_rx; 
	float gain_tx;
	float gain_rx;
};

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
	if (!config_read_file(&cfg, "master_scenario_file.txt"))
	{
		fprintf(stderr, "\n%s:%d - %s", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
		fprintf(stderr, "\nCould not find master scenario file. It should be named 'master_scenario_file.txt'\n");
		config_destroy(&cfg);
		exit(EX_NOINPUT);
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

	// temporary variable to read num_nodes
	int num_nodes;

	// string pointing to scenario file
	char *scenario;
	strcpy(scenario, "scenarios/");
	strcat(scenario, scenario_file);

	// string used to lookup nodes
	char *node;
	config_setting_t *node_config;

	// Read the file. If there is an error, report it and exit.
	if (!config_read_file(&cfg, scenario))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		exit(EX_NOINPUT);
	}

	// Read number of nodes
	config_setting_lookup_int(&cfg, "num_nodes", &num_nodes);

	config_destroy(&cfg);
	return num_nodes;
} // End readScConfigFile()

node_settings_s read_node(int node, char *scenario_file){
	// scenario info struct for node
	struct node_settings_s node_settings = {};

	// lookup specific node
	strcpy(node, "node");
	strcat(node, (char)i);
	node_config = config_lookup(&cfg, node);

	// read CORNET IP address for the node
	if (config_setting_lookup_int(node_config, "CORNET_IP", &tmpI))
		node_settings->CORNET_IP = tmpI;

	// read type of node
	if (config_setting_lookup_int(node_config, "type", &tmpI))
		node_settings->type = tmpI;

	// read all possible node settings
	if (config_setting_lookup_int(node_config, "CRTS_IP", &tmpI))
		node_settings->CRTS_IP = tmpI;

	if (config_setting_lookup_int(node_config, "AP", &tmpI))
		node_settings->AP = tmpI;

	if (config_setting_lookup_int(node_config, "CE", &tmpI))
		node_settings->CE = tmpI;

	if (config_setting_lookup_int(node_config, "PHY_metrics", &tmpI))
		node_settings->PHY_metrics = tmpI;

	if (config_setting_lookup_int(node_config, "MAC_metrics", &tmpI))
		node_settings->PHY_metrics = tmpI;

	if (config_setting_lookup_int(node_config, "NET_metrics", &tmpI))
		node_settings->PHY_metrics = tmpI;

	if (config_setting_lookup_int(node_config, "traffic", &tmpI))
		node_settings->traffic = tmpI;

	if (config_setting_lookup_float(node_config, "freq_tx", &tmpF))
		node_settings->freq_tx = tmpI;

	if (config_setting_lookup_float(node_config, "freq_rx", &tmpF))
		node_settings->freq_rx = tmpF;

	if (config_setting_lookup_float(node_config, "gain_tx", &tmpF))
		node_settings->gain_tx = tmpF;

	if (config_setting_lookup_float(node_config, "gain_rx", &tmpF))
		node_settings->gain_rx = tmpF;

	return node_settings;
}