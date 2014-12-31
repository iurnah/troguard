#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <string>
#include <cstring>

using namespace std;

const char *path = "/home/rui/DrivebyDownload/symbexec/test-cases";

string get_appname(string dirname){
	int start = dirname.find("s2e-out-") + 8;
	int end = dirname.find_first_of("-", start);
	int length = end - start;
	string appname = dirname.substr(start, length);

	return appname;
}

int get_n_args(string line){
	int start = line.find("(int32_t) ") + 10;
	int end = line.find_first_of(",", start);
	int len = end - start;
	string n_args = line.substr(start, len);

	return atoi(n_args.c_str());
}

string get_argv1(string line){
	if (line.find("v1_arg0_1: ") == string::npos){
		return "NULL";
	}
	int start = line.find("(string) \"") + 10;
	int end = line.find_first_of("\"", start);
	int len = end - start;
	string argv1 = line.substr(start, len);

	return argv1;
}

string get_argv2(string line){
	if (line.find("v2_arg1_2: ") == string::npos){
		return "NULL"; 
	}
	int start = line.find("(string) \"") + 10;
    int end = line.find_first_of("\"", start);
	int len = end - start;
	string argv2 = line.substr(start, len);

	return argv2;
}

int convert2replay(char *dirname){
	int count = 0;
	//creat the replay-xxxx.sh output file
	string s_dirname(dirname);
	string app_name = get_appname(s_dirname);
	cout << "converting " << dirname << "...... "<< endl;

	//create the output replay scripts
	string outpath ="/home/rui/DrivebyDownload/symbexec/test-cases/replay/";
	string s_replayscript = outpath + app_name + "-replay" + ".sh";
    const char * replayscript = s_replayscript.c_str();
	ofstream outfile;
	outfile.open(replayscript, std::fstream::app);

	//get the path to the directory of s2e-out-xxx-xxx
	char dirpath[128];
	strcpy(dirpath, path);
	strcat(dirpath, "/");
	strcat(dirpath, dirname);

	//get the path to the file messages.txt	
	char filepath[128];
	strcpy(filepath, dirpath);
	strcat(filepath, "/");
	strcat(filepath, "messages.txt");

	string line;
	ifstream infile(filepath);
	if(!infile) { cout << "Error: could not opend the \"" << filepath << "\" file!" << endl; }

	while(!infile.eof()){
		getline(infile, line);
		if(line.find("v0_n_args_0:") != string::npos){
			int n_args = get_n_args(line);
			string argv1;
			string argv2;
			switch (n_args){
				case 0:
					outfile << app_name << endl;
					count++;
					break;
				case 1:
					if(!infile.eof()){ getline(infile, line); }else continue;
					argv1 = get_argv1(line);
					if(argv1.compare("NULL") != 0){ 
						outfile << app_name << " " << argv1 << endl;
					}
					count++;
					break;
				case 2:
					if(!infile.eof()){ getline(infile, line); }else continue;
					argv1 = get_argv1(line);
					if(!infile.eof()){ getline(infile, line); }else continue;
					argv2 = get_argv2(line);
					if(argv1.compare("NULL") != 0 && argv2.compare("NULL") != 0){ 
						outfile << app_name << " " << argv1 << " " << argv2 <<  endl;
					}
					count++;
					break;
				default:
					continue;
			}
			//outfile << "FOUND the v0_n_args_0" << endl;
		}
		//cout << line << endl;
	}
	cout << count << " " << dirname << " converted!!!" << endl;
	//string command = "sort " + s_replayscript + " | " + "uniq > " + s_replayscript;
	//system(command.c_str());
	//system("sort replayscript | uniq > replayscript");
}

int main(void){
	DIR* dir = opendir(path);
	struct dirent *ent;
	//cout << path << endl;
	if(dir != NULL){
		while((ent = readdir(dir)) != NULL){
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
				continue;
			if (strcmp(ent->d_name, "replay") == 0)
				continue;
			cout << ent->d_name << endl;
			convert2replay(ent->d_name);
		}
		closedir(dir);
	}else {
		perror("Error: ");
		return 1;
	}
}
