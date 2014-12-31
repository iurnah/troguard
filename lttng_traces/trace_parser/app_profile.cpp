#include "common.h"
#include "trace_parser.h"

/*
 * 
 * name: main(){ 
 * @param
 * @return
 * 
 */
int main(int argc, char *argv[]){
	
	if(argc != 3){
		cout << "Usage:: ./app_profile <filename> detection_point_line_number\n\tdetection_piont_line_number=-1 if no stopping point. " << endl;
		return 0;
	}
		
	std::vector <node *> graph;
	std::vector <graph_edge *> edges;
	graph_edge *detection_point_graph_edge = trace_parser(argv, graph);
	
	print_nodes(graph);
	
	//cout << "graph[0]->name:: " << graph[0]->name << " ::graph[0]->type:: " << graph[0]->type << " ::path:: " << graph[0]->path << " ::Children:: " << graph[0]->parents[0]->src_node->name << endl; 
	//cout << "graph[1]->name:: " << graph[1]->name << " ::graph[1]->type:: " << graph[1]->type << " ::path:: " << graph[1]->path << " ::Children:: " << graph[1]->children[0]->dest_node->name << endl; 
	//cout << "graph[0]->name:: " << graph[0]->name << " ::graph[0]->type:: " << graph[0]->type << " ::path:: " << graph[0]->path << " ::parents src_node:: " << graph[0]->parents[0]->src_node->name << 
	//" ::parents dest_node:: " << graph[0]->parents[0]->dest_node->name << " ::children src_node:: " << graph[0]->children[0]->src_node->name << " ::children dest_node:: " << graph[0]->children[0]->dest_node->name << endl; 
	
	
	//" " << graph[0]->parents[0]->src_node->name << " " << graph[0]->parents[0]->dest_node->name << endl;

	//graph[0]-
	
	
	/*
	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			cout << "The nodes's children's dest_node name: " << graph[i]->children[j]->dest_node->name << endl;
			cout << "The nodes's children's src_node name: " << graph[i]->children[j]->src_node->name << endl;
		}
		for (unsigned int j=0; j<graph[i]->parents.size(); j++){
			cout << "The nodes's parents' dest_node name: " << graph[i]->parents[j]->dest_node->name << endl;
			cout << "The nodes's parents' src_node name: " << graph[i]->parents[j]->src_node->name << endl;
		}
		
	}
	*/
	//print_edges(edges);
	return 0;
}
