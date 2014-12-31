//#include <fstream>
//#include <iostream>
#include <string>

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include "common.h"
#include "trace_parser.h"

using namespace std;

int main()
{
/*	string browsers[] = {"chrome","firefox","opera","epiphany","midori", "chromium", "netsurf", "arora", "xxxterm", "rekonq",
						 "elinks", "chimera2", "hbro", "netrik", "links2", "links", "konqueror", "surf", "seamonkey", "Dooble"};

	string offices[] = {"kile","geany","texmaker","calligrawords","soffice.bin", "lyx", "tea", "jed", "emacs", "vi",
						"xemacs", "texmacs", "abiword", "aoeui", "pico", "editra","x2", "scite", "efte", "gedit"};

	string games[] = {"sol","neverball", "wesnoth", "glchess" ,"kmahjongg" ,"supertuxkart" ,"hedgewars" ,"pingus" , "frozen-bubble", "eboard",
					 "supertux", "mines", "sdl-ball", "kpat", "gweled", "hex-a-hop", "blobby", "palapeli", "bovo","xmoto"};

    string ides[] = {"anjuta","codelite" ,"codeblocks" ,"netbeans", "monodevelop", "kdevelop", "spyder", "monkeystudio", "drracket", "idle"}; // 10

	string ims[] = {"skype","kmess" ,"emesene" ,"kopete" ,"pidgin" ,"psi" ,"gajim" ,"empathy" ,"amsn" ,"qutim"};

	string graph[] = {"gimp","pinta" ,"imagej" ,"inkscape" ,"kolourpaint", "rawtherapee", "mypaint", "gpaint", "gnome-paint", "pencil"};

	string players[] = {"smplayer","vlc" ,"audacious" ,"quodlibet" ,"gmusicbrowser" ,"qmmp" ,"abraca" ,"amarok","guayadeque" ,"aqualung"};

	string video_editor[] = {"openshot","lives" ,"iriverter" ,"kino" ,"pitivi" ,"videocut" ,"winff" ,"arista-gtk","kdenlive" ,"curlew"};
	
	string audio_editor[] = {"audacity","avidemux","dvbcut" ,"oggconvert" ,"kwave" ,"wavbreaker" ,"mp3splt-gtk" ,"mhwaveedit","fillmore" ,"soundconverter"};
	
	string calculator[] = {"grpn", "gcalctool", "EdenMath", "speedcrunch", "kcalc", "keurocalc", "extcalc", "gip", "galculator", "gnome-genius", "qalculate-gtk"};
*/
/*
	string app_list[] = {"chrome","firefox","opera","epiphany","midori", "chromium", "netsurf", "arora", "xxxterm", "rekonq",
						"elinks", "chimera2", "hbro", "netrik", "links2", "links", "konqueror", "surf", "seamonkey", "Dooble", //extra 
						"kile", "geany", "texmaker", "calligrawords", "soffice.bin", "lyx", "tea", "jed", "emacs", "vi",
						"xemacs", "texmacs", "abiword", "aoeui", "pico", "editra","x2", "scite", "efte", "gedit",//extra
						"sol", "wesnoth", "glchess", "neverball", "kmahjongg", "supertuxkart" ,"hedgewars" ,"pingus" , "frozen-bubble", "eboard",
						"supertux", "mines", "sdl-ball", "kpat", "gweled", "hex-a-hop", "blobby", "palapeli", "bovo","xmoto", //extra
						"anjuta","codelite" ,"codeblocks" ,"netbeans", "monodevelop", "kdevelop", "spyder", "monkeystudio", "drracket", "idle",
						"skype","kmess" ,"emesene" ,"kopete" ,"pidgin" ,"psi" ,"gajim" ,"empathy" ,"amsn" ,"qutim",
						"gimp","pinta" ,"imagej" ,"inkscape" ,"kolourpaint", "rawtherapee", "mypaint", "gpaint", "gnome-paint", "pencil",
						"smplayer","vlc" ,"audacious" ,"quodlibet" ,"gmusicbrowser" ,"qmmp" ,"abraca" ,"amarok","guayadeque" ,"aqualung",
						"openshot","lives" ,"iriverter" ,"kino" ,"pitivi" ,"videocut" ,"winff" ,"arista-gtk","kdenlive" ,"curlew",
						"audacity","avidemux","dvbcut" ,"oggconvert" ,"kwave" ,"wavbreaker" ,"mp3splt-gtk" ,"mhwaveedit","fillmore" ,"soundconverter",
						"grpn", "gcalctool", "EdenMath", "speedcrunch", "kcalc", "keurocalc", "extcalc", "gip", "galculator", "gnome-genius"}; //"qalculate-gtk"
*/

	string app_list[] = { "date", "arch", "nproc", "uname", "hostname", "hostid", "uptime", //type 21
						  "id", "logname", "whoami", "users", "who", //type 20
						  "ln", "link", "mkdir", "mknod", "mkfifo", //type 12
						  "ls", "dir", "vdir" }; //type 10
					
	//string data_point[] = {"1","2","3","4","5","6"};
	string data_point[] = {"0","1","2","3","4","5"};
	string line, attribute_name, arff_file_path;
	
	ofstream datasets("../arff/coreutils_20.arff");
/*
class attribute_node{
public:
	string attribute_name, attribute_line, appclass, datapoint;
	attribute_node(){};
	attribute_node(string name, string line){
		attribute_name = name;
		attribute_line = line;	
	};		
};
*/
	std::vector <attribute_node *> all_attribute;
	//ifstream attr_declaration_in("/home/rui/DrivebyDownload/symbexec/arff/attribute_declaration.arff");
	ifstream attr_declaration_in("/home/rui/DrivebyDownload/symbexec/arff/attribute_declaration.arff");

	if(!attr_declaration_in){
		cout << "Error: could not open the attribute declaration file!" << endl;
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
	//datasets << "@relation linuxapp" << endl << "\n" << endl;
	datasets << "@relation coreutils" << endl << "\n" << endl;
	//datasets << "@attribute data_point_num { 1, 2, 3, 4, 5, 6 }" << endl;
	//datasets << "@attribute class { " << "chrome, firefox, opera, epiphany, dillo, kile, geany, okular, soffice.bin-d, soffice.bin-w, kmail, thunderbird, evolution, sol, wesnoth, glchess, neverball, kmahjongg " << "}" << endl;
	//datasets << "@attribute class { browser, office, im, game, ide, player, graph, veditor, aeditor, calculator }" << endl;
	datasets << "@attribute class { dirlist, filetype, userinfo, systeminfo }" << endl;
	
//this for loop is used for print the declaration of all_attribute in the datasets.
	for(unsigned int i=0;i<all_attribute.size();i++){
		if(all_attribute[i]->attribute_line.find("Dep(") != string::npos
			|| all_attribute[i]->attribute_line.find("Port(") != string::npos
			|| all_attribute[i]->attribute_line.find("W(") != string::npos
			|| all_attribute[i]->attribute_line.find("R(") != string::npos
			|| all_attribute[i]->attribute_line.find("KB_") != string::npos
			|| all_attribute[i]->attribute_line.find("Mouse_") != string::npos ){
			datasets << "@attribute " << all_attribute[i]->attribute_name << " { true, false }" <<endl;	   
		}else 
			datasets << "@attribute " << all_attribute[i]->attribute_name << " numeric " <<endl;	   
	}

	datasets << "\n" << endl;
	datasets << "@data" << endl;
//start to read each file in the sub-directory. 
	for(int index=0;index<20;index++){
	//the loop for iteration of all the folders	
		//for(int point=0;point<6;point++){
		//the loop for interation of all the files	
		//datasets << "Enter the for loop" << endl;
			
			std::vector <attribute_node *> data_attribute;
			arff_file_path = "/home/rui/DrivebyDownload/symbexec/arff/"+app_list[index]+".arff";
			cout << arff_file_path << endl;
			
			ifstream data_arff_in(arff_file_path.c_str());
			if(!data_arff_in){
				cout << "Error: could not open the .arff file!" << endl;
				exit(1);
			}
			cout << "open the arff files" << endl;

			//for print the appliction type
			string type;
			if(app_list[index] == "ls" || app_list[index] == "dir" || app_list[index] == "vdir"){
				type = "dirlist";
			}else if(app_list[index] == "ln" || app_list[index] == "link" || app_list[index] == "mkdir" || app_list[index] == "mknod" || app_list[index] == "mkfifo"){
				type = "filetype";
			}else if(app_list[index] == "id" || app_list[index] == "logname" || app_list[index] == "whoami" || app_list[index] == "groups" || app_list[index] == "users" || app_list[index] == "who" ){		
				type = "userinfo";
			}else if(app_list[index] == "date" || app_list[index] == "arch" || app_list[index] == "nproc" || app_list[index] == "uname" || app_list[index] == "hostname" || app_list[index] == "hostid" || app_list[index] == "uptime"){
				type = "systeminfo";
			}
			
			cout << "after the class resolve" << endl;
		
			datasets << type << ",";
			//datasets << data_point[point] + "," << app_list[index] + ","; 
			
			cout << "before the while" << endl;
			//using while loop to read each data file such as "chrome_1.arff"
			while(!data_arff_in.eof()){
				getline(data_arff_in, line);
				attribute_node* class_datapoint_node;
				if(line != ""){
					class_datapoint_node = new attribute_node(get_attr_name(line),line);
					class_datapoint_node->appclass = app_list[index];
					//class_datapoint_node->datapoint = data_point[point];
					data_attribute.push_back(class_datapoint_node);	
				}
			}
			
			cout << "after read the attribute" << endl;
			string attribute_value;
			for(unsigned int all=0;all<all_attribute.size();all++){
				attribute_value = search_attribute(data_attribute, all_attribute[all]->attribute_line); 
				cout << "attribute" <<endl;
				//datasets << all << endl;
				//cout << all_attribute[all]->attribute_name << endl;
				datasets << attribute_value << ",";	
			}
			datasets << endl;
			//datasets << "Print out each line" << endl;
		//to match for(int point=0;point<6;point++){
	}

	datasets.close();
	return 0;
 }
