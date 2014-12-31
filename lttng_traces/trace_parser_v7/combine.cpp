#include <fstream>
//#include <iostream>
#include <string>

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "common.h"
#include "trace_parser.h"

using namespace std;

int main()
{
	string app_list[] = {"chrome","firefox","opera","epiphany","dillo","kile","geany","okular","soffice.bin-d","soffice.bin-w","kmail","thunderbird","evolution","sol","wesnoth","glchess","neverball","kmahjongg"};
	string data_point[] = {"1","2","3","4","5","6"};
	string line, attribute_name, arff_file_path;
	
	ofstream datasets("./datasets/linuxapps.arff");
	
	std::vector <attribute_node *> all_attribute;
	ifstream attr_declaration_in("./datasets/attribute_declaration.arff");

	if(!attr_declaration_in){
		cout << "Error: could not open the file!" << endl;
		exit(1);
	}
	//using while loop to read all the
	while(!attr_declaration_in.eof()){
		getline(attr_declaration_in, line);
		attribute_node* tmp_node;
		if(line != ""){
			tmp_node = new attribute_node(get_attr_name(line),line);
			all_attribute.push_back(tmp_node);	
		}
	}
//until now, the all the attribute has been read into the data structure.
	datasets << "@relation linuxapp" << endl << "\n" << endl;
	//datasets << "@attribute data_point_num { 1, 2, 3, 4, 5, 6 }" << endl;
	//datasets << "@attribute class { " << "chrome, firefox, opera, epiphany, dillo, kile, geany, okular, soffice.bin-d, soffice.bin-w, kmail, thunderbird, evolution, sol, wesnoth, glchess, neverball, kmahjongg " << "}" << endl;
	datasets << "@attribute class { " << "browser, office, email, game" << " }" << endl;
	
//this for loop is used for print the declaration of all_attribute in the datasets.
	for(int i=0;i<all_attribute.size();i++){
		if(all_attribute[i]->attribute_line.find("real") != string::npos){
			datasets << "@attribute " << all_attribute[i]->attribute_name << " real" <<endl;	   
		}else if(all_attribute[i]->attribute_line.find("numeric") != string::npos){ 
			datasets << "@attribute " << all_attribute[i]->attribute_name << " numeric" <<endl;
		}else 
			datasets << "@attribute " << all_attribute[i]->attribute_name << " { true, false }" <<endl;	   
	}

	datasets << "\n" << endl;
	datasets << "@data" << endl;
//start to read each file in the sub-directory. 
	for(int index=0;index<18;index++){
	//the loop for iteration of all the folders	
		for(int point=0;point<6;point++){
		//the loop for interation of all the files	
		//datasets << "Enter the for loop" << endl;
			std::vector <attribute_node *> data_attribute;
			arff_file_path = "./results/"+app_list[index]+"/"+app_list[index]+"_"+data_point[point]+".arff";
			//cout << arff_file_path << endl;
			ifstream data_arff_in(arff_file_path.c_str());
			//for print the appliction type
			string type;
			if(app_list[index] == "firefox" || app_list[index] == "chrome" || app_list[index] == "opera" || app_list[index] == "dillo" || app_list[index] == "epiphany"){
				type = "browser";
			}else if(app_list[index] == "kile" || app_list[index] == "geany" || app_list[index] == "okular" || app_list[index] == "soffice.bin-d" || app_list[index] == "soffice.bin-w"){
				type = "office";
			}else if(app_list[index] == "kmail" || app_list[index] == "thunderbird" || app_list[index] == "evolution"){
				type = "email";
			}else if(app_list[index] == "sol" || app_list[index] == "wesnoth" || app_list[index] == "glchess" || app_list[index] == "neverball" || app_list[index] == "kmahjongg"){
				type = "game";
			}
			
			datasets << type << ",";
			//datasets << data_point[point] + "," << app_list[index] + ","; 
			
			if(!data_arff_in){
				cout << "Error: could not open the file!" << endl;
				exit(1);
			}
			//using while loop to read each data file such as "chrome_1.arff"
			while(!data_arff_in.eof()){
				getline(data_arff_in, line);
				attribute_node* class_datapoint_node;
				if(line != ""){
					class_datapoint_node = new attribute_node(get_attr_name(line),line);
					class_datapoint_node->appclass = app_list[index];
					class_datapoint_node->datapoint = data_point[point];
					
					data_attribute.push_back(class_datapoint_node);	
				}
			}
			/*
			for(int i=0;i<data_attribute.size();i++){
				datasets << data_attribute[i]->attribute_name << endl;
				//datasets << get_attr_value(data_attribute[i].attribute_line) << "," << endl;
			}
			*/
			
			string attribute_value;
			for(int all=0;all<all_attribute.size();all++){
				attribute_value = search_attribute(data_attribute, all_attribute[all]->attribute_line); 
				//datasets << all << endl;
				//cout << all_attribute[all]->attribute_name << endl;
				datasets << attribute_value << ",";	
			}
			datasets << endl;
			//datasets << "Print out each line" << endl;
		}//to match for(int point=0;point<6;point++){
	}


	datasets.close();
	return 0;
  }
