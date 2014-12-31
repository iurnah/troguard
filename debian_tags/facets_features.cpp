#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <unistd.h>

using namespace std;

string get_type(string line){
	int index_of_string_start = 0;
	int index_of_string_end = line.find(" | ");
	int attr_name_length = index_of_string_end;
	string type = line.substr(index_of_string_start, attr_name_length);	
	//cout << type << endl;
	return type; 
}

vector <string> get_facets(string line){

	vector <string> facets_vec;
	int istart = 0;
	int iend = 0;
	int length = 0;
	string facet;

	while(1){
		istart = line.find(" | ", istart);
		//cout << istart << endl;
		iend = line.find(" | ", istart+3);
		//cout << iend << endl;
		facet = line.substr(istart+3, iend - istart-3);
		//cout << facet << endl;
		facets_vec.push_back(facet);

		if(iend != string::npos){
			istart = iend;
			//cout << "in while loop" << endl;
 		}else{
			break;
		}
	}

	return facets_vec;
}

string get_app_name(string line){
	int index_of_string_start = 0;
	int index_of_string_end = line.find(" ");
	int attr_name_length = index_of_string_end;
	string app_name = line.substr(index_of_string_start, attr_name_length-1);	

	return app_name; 
}

string truncate_line(string line){
	int istart = line.find(": ")+2;
	string attributes = line.substr(istart) ;

	return attributes;
}

int main(int argc, char **argv){
	if(argc != 3){
		cout << "check the input argument!" << endl;
		exit(1);
	}

	ofstream named_app("named_app.dat");
	ofstream app_list("app_list.dat");

	//read the type_def.dat file
	ifstream file_type_def(argv[1]);
	//cout << argv[1] << argv[2];
	if(!file_type_def){ cout << "Error: could not open the file!" << endl; exit(1); }

	map<string, vector<string> > type_def_map;
	map<string, vector<string> >::iterator map_it;
	
	string app_type;
	vector<string> facets_vec;
	vector<string>::iterator vec_it;
	
	string line;
	while(getline(file_type_def, line)){
		app_type = get_type(line);
		//cout << "app_type::" << app_type << endl;
		facets_vec = get_facets(line);

		type_def_map.insert(pair<string, vector<string> > (app_type, vector<string>()));
		//cout << "facetes_vec.size() = " << facets_vec.size() << endl;
		
		for(vec_it = facets_vec.begin(); vec_it != facets_vec.end(); ++vec_it){
			//cout << "*vec_it" << *vec_it << endl;
			type_def_map[app_type].push_back(*vec_it);
		}
	}

	//cout << "type_def_map.size() = " << type_def_map.size() << endl;

	for(map_it = type_def_map.begin(); map_it != type_def_map.end(); ++map_it){
		//cout << " map_it->first:: " << map_it->first << endl;
		for(vec_it = map_it->second.begin(); vec_it != map_it->second.end(); ++vec_it){
			//cout << "**vec_it:: " << *vec_it << endl;
		}
	}

	ifstream file_tagdb(argv[2]);
	if(!file_tagdb){ cout << "Error: could not open tagdb file!" << endl; exit(1); }

	int i, facet_count;
	int static_facet_count = 0;
	
	//string true_type;
	string static_true_type;
	//repeat:
	while(getline(file_tagdb, line)){
		//cout <<  i++ << "in while loop" << endl;
		if(line.find("role::program") == string::npos){ continue; }
		static_facet_count = 0;
		static_true_type = "UNKNOW:";
		if(line.find("web::browser,") != string::npos){
			named_app << "browser:, " << truncate_line(line) << "," << endl;
			app_list << get_app_name(line) << endl;
			continue;
		}
		
		for(map_it = type_def_map.begin(); map_it != type_def_map.end(); ++map_it){
			facet_count = 0;
			for(vec_it = map_it->second.begin(); vec_it != map_it->second.end(); ++vec_it){
				if(line.find(*vec_it) != string::npos){
					facet_count++;	
				}//end of if
				//cout << facet_count << endl;
			}//inner for
			//break;
			//cout << facet_count << endl;

			
			if(facet_count > static_facet_count){
				static_facet_count = facet_count;
				static_true_type = map_it->first;				
			}else if(facet_count = static_facet_count){
				//cout << "cannot distinguish class" << endl;
			}
			//cout << static_true_type << endl;	
		}//outer for
		if(static_true_type != "UNKNOW:"){
			named_app << static_true_type << ", " << truncate_line(line) << "," << endl;
			app_list << get_app_name(line) << endl;
		}
	}//while

	//TODO1: how to resolve multipel match in a line of the database file?
	//solution: count the facets apperance rate in each line of the data base filed
	// 			and find the maximum in the meantime. 
	//TODO2: combine named_app.dat and the infor extracted from control file. 
	
}
