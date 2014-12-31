#include "common.h"
#include "dependency_graph.cpp"
#include "VulDB.cpp"
#include "AGT.cpp"
#include "forensics.cpp"
#include "Bayesian_network.cpp"
#include "print.cpp"
#include "syslog_parser.cpp"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>


void stdio_print(std::vector <node *> graph){
	for (unsigned int i=0; i<graph.size(); i++) 
		cout << i << ": " << graph[i]->name << " " << graph[i]->type << endl; 
}

// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&   main function   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

int main(int argc, char *argv[]){
		
	//system("cp /var/log/messages .; rm *.eps;");
	if (argc != 3){
		cout << "Usage:: ./log_parser filename detection_point_line_number\n\tdetection_point_line_number=-1 if no stopping point. " << endl;
		return 0;
	}
	
	
	
	std::vector <node *> graph;
	graph_edge *detection_point_graph_edge = syslog_parser(argv, graph);
	
	socket_IO_mapper(graph);
	
	//graph2dot(graph);
	
	/*
	// begin.saman.bayesian test
	
	cout << "Graph refinement... graph.size()=" << graph.size() << endl;
	refine_graph(graph);
	cout << "done. graph.size()=" << graph.size() << endl;
	
	//stdio_print(graph);
	//graph2dot(graph);
	graph2CDG2dot(graph, false);
	
	// removing sockets and nodes with more than 20 parents.
	for (unsigned int i=0; i<graph.size(); i++){
		if (graph[i]->parents.size() > 18){
			cout << "\nLOOK:: removing.. " << graph[i]->name << " parent_num:" << graph[i]->parents.size() << endl;
			remove_graph_node(graph, i);
			i = 0;
		}
	}
	
	cout << "flow_freq.dat REPORT..." << endl;
	std::vector <int> stats;
	unsigned int max = 1;
	for (unsigned int i=0; i<graph.size(); i++)
		for (unsigned int j=0; j<graph[i]->children.size(); j++)
			if (graph[i]->children[j]->times.size() > max)
				max = graph[i]->children[j]->times.size();
	max++;
	cout << "MAX=" << max << endl;
	int dist[max];
	for (unsigned int i=0; i<max; i++) dist[i]=0;
	for (unsigned int i=0; i<graph.size(); i++)
		for (unsigned int j=0; j<graph[i]->children.size(); j++)
			dist[graph[i]->children[j]->times.size()]++;
	
	ofstream of11("flow_freq.dat");
	for (unsigned int i=1; i<max; i++){
		of11 << i << "\t" << dist[i] << endl;
	}
	of11.close(); 
	cout << "REPORT DONE." << endl;
	
	// putting evidences.
	set_evidence(graph); // clear all nodes
	set_evidence(graph, "SOCK_AF_INET"); 
	//set_evidence(graph, "apache2"); 
	
	cout << "Creating Bayesian Network... graph_size:" << graph.size() << endl;
	directed_graph<bayes_node>::kernel_1a_c *bn = Bayesian_network(graph);
	
	cout << "Starting Bayesian Inference... graph_size:" << graph.size() << endl;
	Bayesian_Inference(bn, graph, 1000);
	for (unsigned int i=0; i<graph.size(); i++) if (fabs(graph[i]->probability) > 0.00001) cout << i << " " << graph[i]->name << " " << graph[i]->probability << endl;
	return 0;
	// end.saman
	*/
	
	node *detection_point_process = (detection_point_graph_edge->src_node->type == PROCESS) ? 
										detection_point_graph_edge->src_node : detection_point_graph_edge->dest_node;
	
	pair <graph_edge *, int> detection_point(detection_point_graph_edge, detection_point_graph_edge->times.size()-1);
	
	int counter = 0;
	for (unsigned int i=0; i<graph.size(); i++) if (graph[i]->type == PROCESS  ||  graph[i]->type == REG) counter++;
	cout << "\nSaman:: before refine_graph num_nodes: " << graph.size() << "  counter:" << counter << endl;
	
	refine_graph(graph);
	
	counter = 0;
	for (unsigned int i=0; i<graph.size(); i++) if (graph[i]->type == PROCESS  ||  graph[i]->type == REG) counter++;
	cout << "Saman:: after refine_graph num_nodes: " << graph.size() << "  counter (process or reg):" << counter << endl;

	//node_behavoir_estimator(graph, false);
	//node2dist(graph, "corehttp");

	
	/*
	for (int i=0; i<graph.size(); i++){
		
		int parent_counter = 0;
		for (int j=0; j<graph.size(); j++){
			for (int k=0; k<graph[j]->children.size(); k++)
				if (graph[j]->children[k]->dest_node == graph[i])
					parent_counter++;
		}
		
		if (parent_counter != graph[i]->parents.size()){
			cout << "Error:: " << graph[i]->parents.size() << " " << parent_counter << endl;
			return 0;
		}
	}
	*/
	
	cout << "\nSaman:: Calling reachability analysis..." << endl;
	
	graph2CDG2dot(graph);
	reachability_analysis(graph, detection_point);
	
	cout << endl;
	int fifo1 =0, inet1 = 0, unix1 = 0, reg1 = 0, ipcmsg1 = 0, process1 = 0;
	
	for (unsigned int i=0; i<graph.size(); i++){
		if (graph[i]->type != FIFO && graph[i]->type != SOCK_AF_UNIX && graph[i]->type != SOCK_AF_INET  &&  graph[i]->type != IPCMSG  && graph[i]->type != REG  &&  graph[i]->type != PROCESS)
			cout << "new type: " << graph[i]->type << endl;
			
		if (graph[i]->type == FIFO) fifo1++;
		if (graph[i]->type == SOCK_AF_UNIX) unix1++;
		if (graph[i]->type == SOCK_AF_INET) inet1++;
		if (graph[i]->type == REG) reg1++;
		if (graph[i]->type == IPCMSG) ipcmsg1++;
		if (graph[i]->type == PROCESS) process1++;
	}
	cout << "FIFO:" << fifo1 << " AF_UNIX:" << unix1 << " AF_INET:" << inet1 << " REG:" << reg1 << " IPCMSG:" << ipcmsg1 << " PROC:" << process1 << endl;
	
	
	
	//logs to graph.dot and generates eps
	//graph2dot(graph);  
	
	//generates distribution for a node (eps file)
	//node_behavoir_estimator(graph);
	//node2dist(graph, "corehttp");
	
	//logs to graph.dot and generates eps
	//graph2CDG2dot(graph);
	
	// reading/parsing the vuldb
	VulDB *vuldb = VulDB_parser("VulDB.dat");
	
	// generating AGT
	//std::vector <AGT_node *> AGT;
	AGT *agt = new AGT();
	build_AGT(graph, detection_point_process, agt, vuldb);
	cout << "agt_size:" << agt->nodes.size() << " " << detection_point_process->name << endl;
	
	// automated forensics
	automated_forensics(vuldb, agt);
	
	//logs to agt.dot and generates eps
	AGT2dot(vuldb, agt); 
	
	return 0;
}

