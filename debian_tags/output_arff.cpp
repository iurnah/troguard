#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stdio.h>
#include <unistd.h>

using namespace std;

set <string> get_attributes_sets(string line){

	set <string> facets_set;
	int istart = 0;
	int iend = 0;
	int length = 0;
	string facet;
	//cout << line << endl;

//remove the last comma
	int last = line.find_last_of(",");
	line = line.substr(0, last);

	istart = line.find(":, ");
	while(1){			
		istart = line.find(", ", istart);
		//cout << istart << endl;
		iend = line.find(", ", istart+2);
		//cout << iend << endl;
		facet = line.substr(istart+2, iend - istart-2);
		//cout << facet << endl;
		facets_set.insert(facet);

		if(iend != string::npos){
			istart = iend;
			//cout << "in while loop" << endl;
 		}else{
			break;
		}
	}

	return facets_set;
}

string get_app_type(string line){
	int istart = 0;
	int iend = line.find(":, ") + 2;
	int len = iend;
	string type = line.substr(istart, len);	

	return type;
}

int main(int argc, char **argv){

	if(argc != 2){ //input including the "all_tag_control.dat" file
		cout << "check the input argument!" << endl;
		exit(1);
	}

	ofstream arff_header("arff_header.arff");
	ofstream arff_value("arff_value.arff");

	//read the all_tag_control.dat file
	ifstream all_tag_control(argv[1]);
	if(!all_tag_control){ cout << "Error: could not open the file!" << endl; exit(1); }
	
	string line;
	set<string> attributes_set;
	set<string>::iterator attributes_set_it;

	set<string> all_attributes_set;
	set<string>::iterator all_attributes_set_it;
	
	//string line;
	//debug
	//cout << "it's OK" << endl; 
	
	//This while is to get all the attributes stored in a single data structure
	while(getline(all_tag_control, line)){
		attributes_set = get_attributes_sets(line);
		for(attributes_set_it = attributes_set.begin(); attributes_set_it != attributes_set.end(); ++attributes_set_it){
			all_attributes_set.insert(*attributes_set_it);
		}
	}
	//print the header section of the arff file
	arff_header << "@relation debian_pkg_3709" << endl;
	arff_header << "@attribute class { browser:, email:, game:, message:, editor:, security:, graphics:, download:, reader:, media: }" << endl;
	for(all_attributes_set_it = all_attributes_set.begin(); all_attributes_set_it != all_attributes_set.end(); ++all_attributes_set_it){
		arff_header << "@attribute " << *all_attributes_set_it << "{ true, false }" << endl;
	}
	arff_header << "@attribute Install-Size numeric" << endl;
	arff_header << "@attribute Size numeric" << endl;
	arff_header << "@data" << endl;

	all_tag_control.close();
	
	ifstream all_tag_control_2(argv[1]);
	while(getline(all_tag_control_2, line)){
		//cout << "in the second while loop" << endl;
		arff_value << get_app_type(line);
		//cout << get_app_type(line) << endl;
	 
		for(all_attributes_set_it = all_attributes_set.begin(); all_attributes_set_it != all_attributes_set.end(); ++all_attributes_set_it){
			if(line.find(*all_attributes_set_it) != string::npos)
				arff_value << "true,";
			else
				arff_value << "false,";
		}
		arff_value << endl;
	}

}
