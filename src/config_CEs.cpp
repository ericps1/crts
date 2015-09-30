#include <string>
#include <string.h>
#include <vector>
#include <fstream>
#include <dirent.h>
#include "node_parameters.hpp"
#include "read_configs.hpp"

int main(){

    // Read in all file names in cognitive_engines directory.
    // Count number of CEs and truncate '.cpp' from file name.
    int num_ces = 0;
    std::string ce_list[100];
    DIR *dpdf;
    struct dirent *epdf;

    dpdf = opendir("./cognitive_engines");
    if (dpdf != NULL){
        while ((epdf = readdir(dpdf))){
            if(epdf->d_name[0]!='.' ){
                // Copy filename into list of CE names
                ce_list[num_ces].assign(epdf->d_name);
                // Strip the extension from the name
                std::size_t dot_pos = ce_list[num_ces].find(".");
                ce_list[num_ces].resize(dot_pos);
                num_ces++;
            }
        }
    }

    printf("Configuring CRTS to use the following cognitive engines:\n");
    for(int i=0; i<num_ces; i++)
        printf("%s\n", ce_list[i].c_str());

    // create string vector
    std::vector<std::string> file_lines;
    std::string line;
    std::string flag_beg = "EDIT START FLAG";
    std::string flag_end = "EDIT END FLAG";
    bool edit_content = false;
    
    //////////////////////////////////////////////////////////////////////////////////
    // Edit ECR.cpp

    // open read file
    std::ifstream file_in("src/ECR.cpp", std::ifstream::in);

    // read file until the end
    while(!(file_in.eof())){

        std::getline(file_in, line);
        if(!edit_content){
            file_lines.push_back(line);                // push all lines on vector
            std::size_t found = line.find(flag_beg);
            if(found != std::string::npos){
                edit_content = true;

                // push all lines to map subclass
                for(int i=0; i<num_ces; i++){
                    line = "    if(!strcmp(ce, \"" + ce_list[i] + "\"))\r";
                    file_lines.push_back(line);
                    line = "        CE = new " + ce_list[i] + "();\r";
                    file_lines.push_back(line);
                }
            }
        }
        // else delete all lines until end flag is found
        else{
            // if end flag is found push it onto the vector and go back to standard mode
            std::size_t found = line.find(flag_end);
            if(found != std::string::npos){
                file_lines.push_back(line);
                edit_content = false;
            }
        }
    }
    // close read file
    file_in.close();
    
    // write file
    std::ofstream file_out("src/ECR.cpp", std::ofstream::out);
    for(std::vector<std::string>::iterator i=file_lines.begin(); i!=file_lines.end(); i++){
        file_out << (*i);
        if(i!=file_lines.end()-1)
            file_out << '\n';
    }
    file_out.close();

    /////////////////////////////////////////////////////////////////////////////////////
    // Edit CE.hpp

    file_lines.clear();

    // open header file
    file_in.open("include/CE.hpp", std::ifstream::in);

    // read file until the end
    while(!(file_in.eof())){

        std::getline(file_in, line);
        if(!edit_content){
            file_lines.push_back(line);                // push all lines on vector
            std::size_t found = line.find(flag_beg);
            // start to edit content once start flag is found
            if(found != std::string::npos){
                edit_content = true;
                
                // push all lines to map subclass
                std::string line_new;
                for(int i=0; i<num_ces; i++){
                    line_new = "class " + ce_list[i] + " : public Cognitive_Engine {\r";
                    file_lines.push_back(line_new);
                    line_new  = "public:\r";
                    file_lines.push_back(line_new);
                    line_new = "    " + ce_list[i] +"();\r";
                    file_lines.push_back(line_new);
                    line_new = "    ~" + ce_list[i] + "();\r";
                    file_lines.push_back(line_new);
                    line_new = "    virtual void execute(void * _args);\r";
                    file_lines.push_back(line_new);
                    line_new = "    void * custom_members;\r";
                    file_lines.push_back(line_new);
                    line_new = "};\r";
                    file_lines.push_back(line_new);
                }
            }
        }
        // else delete all lines until end flag is found
        else{
            // if end flag is found push it onto the vector and go back to standard mode
            std::size_t found = line.find(flag_end);
            if(found != std::string::npos){
                file_lines.push_back(line);
                edit_content = false;
            }
        }
    }
    // close read file
    file_in.close();

    // write file
    file_out.open("include/CE.hpp", std::ofstream::out);
    for(std::vector<std::string>::iterator i=file_lines.begin(); i!=file_lines.end(); i++){
        file_out << (*i);
        if(i!=file_lines.end()-1)
            file_out << '\n';
    }
    file_out.close();

    ///////////////////////////////////////////////////////////////////////////////////////
    // Edit makfile

    file_lines.clear();
    
    // open header file
    file_in.open("makefile", std::ifstream::in);

    // read file until the end
    while(!(file_in.eof())){

        std::getline(file_in, line);
        if(!edit_content){
            file_lines.push_back(line);                // push all lines on vector
            std::size_t found = line.find(flag_beg);
            // start to edit content once start flag is found
            if(found != std::string::npos){
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
                file_lines.push_back(line_new);
            }
        }
        // else delete all lines until end flag is found
        else{
            // if end flag is found push it onto the vector and go back to standard mode
            std::size_t found = line.find(flag_end);
            if(found != std::string::npos){
                file_lines.push_back(line);
                edit_content = false;
            }
        }
    }
    // close read file
    file_in.close();

    // write file
    file_out.open("makefile", std::ofstream::out);
    for(std::vector<std::string>::iterator i=file_lines.begin(); i!=file_lines.end(); i++){
        file_out << (*i);
        if(i!=file_lines.end()-1)
            file_out << '\n';
    }
    file_out.close();
    

    return 0;
}















