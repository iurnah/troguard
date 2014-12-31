#include "common.h"
#include "trace_parser.h"

int main(int argc, char *argv[]){
	
	if(argc != 2){
		cout << "Usage:: ./app_profile <filename> detection_point_line_number\n\tdetection_piont_line_number=-1 if no stopping point. " << endl;
		return 0;
	}
		 
	std::vector <node *> graph;
	std::vector <node *> simpgraph;
	graph_edge *detection_point_graph_edge = trace_parser(argv, graph); 
	//split_raw_data();
	
	syscall_statis(argv);
	
	//print_nodes(graph);

	//generate_report(graph); //generate a full report and seperate files for each catergories of the files
	
	//graph2dot(graph);// print the dot file for all the node
	
	graph2arff(graph);
	return 0;
}
