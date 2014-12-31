#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set> 
#include <algorithm>

class ids_cap{
public:
	string ids_name;
	float fp_rate, fn_rate;
	
	ids_cap(string ds="default_ids", float fp=1, float fn=1){
		ids_name = ds;
		fp_rate = fp;
		fn_rate = fn;
	};
};

class VulDB{ 
public:
	std::map <string, std::vector <string> > process_vul; // stores possible vuls for each process
	std::map <string, std::vector <ids_cap *> > vul_ids; // stores what IDSes can (with what accuracy) detect each vul_exploitation.
	std::map <string, std::vector<string> > ids_vul; // stores what vul_exploitations can be detected each IDS.
	std::vector <pair <string, float> > idses; // vector of available idses and their individual performance costs.
	std::vector <string> processes; // vector of processes mentioned in the vuldb
	VulDB(){};
};


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&   VulDB-related functions &&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


// Description: VulDB_parser gets the VulDB file and parses, and 
// constructs the necessary data structures.
VulDB *VulDB_parser(const char *filename){
	
	VulDB *vuldb = new VulDB();
	
	ifstream if1(filename);
	if (!if1){
		cout << "Error:: <VulDB_parser>: Could not open the file: " << filename << endl;
		exit(1);
	}
	
	string tmp_str, line;
	while(!if1.eof()){
		
		getline(if1, line);
		if (line.find("begin{Process-Vulnerability}") != string::npos){
			
			while(line != "end{Process-Vulnerability}"){
				
				getline(if1, line);
				if (line.find(':') == string::npos) continue;
				string process = line.substr(0, line.find(':')-1);
				line = line.substr(line.find(':')+2);
				
				vuldb->processes.push_back(process);
				
				istringstream strm(line);
				while (!strm.eof()){
					strm >> tmp_str;
					vuldb->process_vul[process].push_back(tmp_str);
				}
			}
			
			cout << "\nVulDB Report:\n";
			std::map <string, std::vector <string> >::iterator it;
			for (it=vuldb->process_vul.begin(); it!=vuldb->process_vul.end(); it++){
				cout << "\t" << (*it).first << " : ";
				for (unsigned int i=0; i<(*it).second.size(); i++)
					cout << (*it).second.at(i) << " ";
				cout << endl;
			}
			
		}else if (line.find("begin{IDS-Vulnerability(FP,FN)}") != string::npos){
			
			while(line != "end{IDS-Vulnerability(FP,FN)}"){
				
				getline(if1, line);
				if (line.find(':') == string::npos) continue;
				string ids = line.substr(0, line.find(':')-1);
				line = line.substr(line.find(':')+2);
				
				istringstream strm(line);
				
				float ids_cost;
				strm >> ids_cost;
				vuldb->idses.push_back(pair <string, float>(ids, ids_cost));
								
				float fp_rate, fn_rate;
				while (!strm.eof()){
					strm >> tmp_str >> fp_rate >> fn_rate;
					vuldb->vul_ids[tmp_str].push_back(new ids_cap(ids, fp_rate, fn_rate));
					vuldb->ids_vul[ids].push_back(tmp_str);
				}
			}
			
			cout << "\nVulDB Report:\n";
			std::map <string, std::vector <ids_cap *> >::iterator it;
			for (it=vuldb->vul_ids.begin(); it!=vuldb->vul_ids.end(); it++){
				cout << "\t" << (*it).first << " : ";
				for (unsigned int i=0; i<(*it).second.size(); i++)
					cout << (*it).second.at(i)->ids_name << "(" << (*it).second.at(i)->fp_rate << ", "  
							<< (*it).second.at(i)->fn_rate << ")  ";
				cout << endl;
			}
		}
		
	}
	
	return vuldb;
}

