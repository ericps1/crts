#include <string>
#include <string.h>
#include <vector>
#include <fstream>
#include "node_parameters.hpp"
#include "read_configs.hpp"

int main(){

	// read master scenario file
	char scenario_list[30][60];
	int num_scenarios = read_scenario_master_file(scenario_list);		

	int num_ces = 0;
	struct node_parameters np;

	std::string ce_list[100];

	// read scenario files and store CE names
	for(int i=0; i<num_scenarios; i++){
		struct scenario_parameters sp = read_scenario_parameters(&scenario_list[i][0]);		
	
		// read all nodes in scenario
		for(int j=0; j<sp.num_nodes; j++){
			np = read_node_parameters(j+1, &scenario_list[i][0]);			
			//print_node_parameters(&np);			
			bool ce_exists = false;
			// compare ce to previously read ces
			for(int k=0; k<num_ces; k++){
				if(!strcmp(np.CE, ce_list[k].c_str()) || !strcmp(np.CE, "")){
					ce_exists = true;
				}
			}
			if(!ce_exists){
				ce_list[num_ces].assign(np.CE);
				num_ces++;
			}
		}
	}

	//for(int i=0; i<num_ces; i++)
	//	printf("CE: %s\n", ce_list[i].c_str());

	// create string vector
	std::vector<std::string> file_lines;
	std::string line;
	std::string flag_beg = "EDIT START FLAG";
	std::string flag_end = "EDIT END FLAG";
	bool edit_content = false;
	
	// open read file
	std::ifstream file_in("/users/ericps1/crts/src/CR.cpp", std::ifstream::in);

	// read file until the end
	while(!(file_in.eof())){

		std::getline(file_in, line);
		//printf("%s\n", line.c_str());
		if(!edit_content){
			file_lines.push_back(line);				// push all lines on vector
			std::size_t found = line.find(flag_beg);
			if(found != std::string::npos){
			//if(!strcmp(line.c_str(), flag_beg.c_str())){			// start to edit content once start flag is found
				printf("Found cpp start flag!\n");
				edit_content = true;

				// push all lines to map subclass
				for(int i=0; i<num_ces; i++){
					line = "	if(!strcmp(ce, \"" + ce_list[i] + "\"))\r";
					file_lines.push_back(line);
					line = "		CE = new " + ce_list[i] + "();\r";
					file_lines.push_back(line);
				}
			}
		}
		// else delete all lines until end flag is found
		else{
			// if end flag is found push it onto the vector and go back to standard mode
			std::size_t found = line.find(flag_end);
			if(found != std::string::npos){
			//if(!strcmp(line.c_str(), flag_end.c_str())){
				printf("Found cpp end flag!\n");
				file_lines.push_back(line);
				edit_content = false;
			}
		}
	}
	// close read file
	file_in.close();
	
	// write file
	std::ofstream file_out("/users/ericps1/crts/src/CR.cpp", std::ofstream::out);
	for(std::vector<std::string>::iterator i=file_lines.begin(); i!=file_lines.end(); i++){
		file_out << (*i);// + "\r";//.substr(0, (*i).length()-1);
		if(i!=file_lines.end()-1)
			file_out << '\n';
	}
	file_out.close();

/////////////////////////////////////////////////////////////////////////////////////

	file_lines.clear();

	// open header file
	file_in.open("/users/ericps1/crts/include/CE.hpp", std::ifstream::in);

	// read file until the end
	while(!(file_in.eof())){

		std::getline(file_in, line);
		if(!edit_content){
			file_lines.push_back(line);				// push all lines on vector
			std::size_t found = line.find(flag_beg);
			if(found != std::string::npos){
			//if(!strcmp(line.c_str(), flag_beg.c_str())){			// start to edit content once start flag is found
				printf("Found hpp start flag!\n");
				edit_content = true;
				
				// push all lines to map subclass
				std::string line_new;
				for(int i=0; i<num_ces; i++){
					line_new = "class " + ce_list[i] + " : public Cognitive_Engine {\r";
					printf("%s\n", line_new.c_str());
					file_lines.push_back(line_new);
					line_new  = "public:\r";
					printf("%s\n", line_new.c_str());
					file_lines.push_back(line_new);
					line_new = "	" + ce_list[i] +"();\r";
					printf("%s\n", line_new.c_str());
					file_lines.push_back(line_new);
					line_new = "	~" + ce_list[i] + "();\r";
					printf("%s\n", line_new.c_str());
					file_lines.push_back(line_new);
					line_new = "	virtual void execute(void * _args);\r";
					printf("%s\n", line_new.c_str());
					file_lines.push_back(line_new);
					line_new = "	void * custom_members;\r";
					printf("%s\n", line_new.c_str());
					file_lines.push_back(line_new);
					line_new = "};\r";
					printf("%s\n", line_new.c_str());
					file_lines.push_back(line_new);
				}
			}
		}
		// else delete all lines until end flag is found
		else{
			// if end flag is found push it onto the vector and go back to standard mode
			std::size_t found = line.find(flag_end);
			if(found != std::string::npos){
			//if(!strcmp(line.c_str(), flag_end.c_str())){
				printf("Found hpp end flag!\n");
				file_lines.push_back(line);
				edit_content = false;
			}
		}
	}
	// close read file
	file_in.close();

	// write file
	file_out.open("/users/ericps1/crts/include/CE.hpp", std::ofstream::out);
	for(std::vector<std::string>::iterator i=file_lines.begin(); i!=file_lines.end(); i++){
		file_out << (*i);//.substr(0, (*i).length()-1);
		if(i!=file_lines.end()-1)
			file_out << '\n';
	}
	file_out.close();

///////////////////////////////////////////////////////////////////////////////////////

	file_lines.clear();
	
	// open header file
	file_in.open("/users/ericps1/crts/makefile", std::ifstream::in);

	// read file until the end
	while(!(file_in.eof())){

		std::getline(file_in, line);
		if(!edit_content){
			file_lines.push_back(line);				// push all lines on vector
			std::size_t found = line.find(flag_beg);
			if(found != std::string::npos){
			//if(!strcmp(line.c_str(), flag_beg.c_str())){			// start to edit content once start flag is found
				printf("Found makefile start flag!\n");
				edit_content = true;
				
				// push all lines to map subclass
				std::string line_new;
				line_new = "CEs = src/CE.cpp";
				for(int i=0; i<num_ces; i++){
					line_new += " cognitive_engines/";
					line_new += ce_list[i];
					line_new += ".cpp";
				}
				line_new += "\r";
				printf("%s\n", line_new.c_str());
				file_lines.push_back(line_new);
			}
		}
		// else delete all lines until end flag is found
		else{
			// if end flag is found push it onto the vector and go back to standard mode
			std::size_t found = line.find(flag_end);
			if(found != std::string::npos){
			//if(!strcmp(line.c_str(), flag_end.c_str())){
				printf("Found makefile end flag!\n");
				file_lines.push_back(line);
				edit_content = false;
			}
		}
	}
	// close read file
	file_in.close();

	// write file
	file_out.open("/users/ericps1/crts/makefile", std::ofstream::out);
	for(std::vector<std::string>::iterator i=file_lines.begin(); i!=file_lines.end(); i++){
		file_out << (*i);//.substr(0, (*i).length()-1);
		if(i!=file_lines.end()-1)
			file_out << '\n';
	}
	file_out.close();


	return 0;
}















