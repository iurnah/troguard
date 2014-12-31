#include "common.h"
#include "trace_parser.h"

using namespace std;

int main(int argc, char *argv[]){

	if(argc != 2){
		cout << "Usage:: ./app_profile <filename> detection_point_line_number\n\tdetection_piont_line_number=-1 if no stopping point. " << endl;
		return 0;
	} 

	multimap <string, node *> graph;
	//graph_edge *detection_point_graph_edge = trace_parser(argv, graph);
	trace_parser(argv, graph);
	//graph_traversal(graph);
    //graph_traversal_depend(graph);
    //print_depend(graph);
    map <string,string> attribute;
    //map <string, string> attr_grp3;
    map <string,string>::iterator it;
    
    //MOD: for(double i=0;i<6;i++){
		//MOD: string mark = doubleToString(i);
		string out_path = "../arff/"APPNAME".arff";
		
		ofstream arff_file;
		arff_file.open(out_path.c_str());
		cout << out_path << endl;
		//MOD: attribute=get_attribute(graph, argv, i);
		attribute=get_attribute(graph, argv);
		
		for(it=attribute.begin(); it!=attribute.end(); ++it){
			cout << "output: " << it->first << " " << it->second << endl;
			arff_file << "\t@attribute " << it->first << " " << it->second << endl;
		}	
	
	return 0;
}
