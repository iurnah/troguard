/*
 * trace_parser.h
 * 
 * This file is to define the dependency graph that the tracing can form
 * Two main class will be defined: node and graph_edge
 * 
 * 
 * 
 */

#ifndef _TRACE_PARSER_H
#define _TRACE_PARSER_H

#include "common.h"
class node;
/*
 * 
 * name: class graph_edge{
 * @param
 * @return
 * 
 */
class graph_edge{ // used as edges in dependency graph
public:
	node *src_node; // source node of the directed edge
	node *dest_node; // destination node of the directed edge
	
	float src_frequency; // [0,1]: how frequently the src_node uses this edge. (e.g., if frequency=1, src_node writes to only the dest_node)
	float dest_frequency; // [0,1]: how frequently the dest_node uses this edge. (e.g., if frequency=1, dest_node reads from only the src_node)
	
	vector <unsigned long> times; // time series which this (data flow) edge was taken
	
	graph_edge(){
		src_frequency = 0;
		dest_frequency = 0;
	}
	graph_edge(node *src, node *dst){
		src_frequency = 0;
		dest_frequency = 0;
		src_node = src;
		dest_node = dst;
	}
	graph_edge(node *src, node *dst, unsigned long t){
		src_frequency = 0;
		dest_frequency = 0;
		src_node = src;
		dest_node = dst;
		times.push_back(t);
	}
}; 
/*
 * 
 * name: class node{
 * @param
 * @return
 * 
 */
class node{ // used as nodes in dependency graph
public:
	int ID; // this is not set initially. (currently, used in bayesian_network creation.)
	
	string name, type, path;
	
	string pid, 
			seccon, // security context (used for only processes)
			fd, inode_id,
			s_addr, d_addr, // used for AF_INET sockets
			peer_inode_id, // used for AF_UNIX
			nrbufs, curbuf; // used for FIFO objects (named pipes)
		
	vector <graph_edge *> children; // children of the node in directed graph
	vector <graph_edge *> parents; // parents of the node in directed graph
	
	node(){};
	node(string type_a, string  name_a){
		name = name_a;
		type = type_a;
	};
	
	// the following variables are used for analyses
	
	// bayesian inference on the bayesian network (dependency graph) for Seclius.
	bool evidence; // if this node is evidence (tainted based on the state variables)
	float probability; // probability of this node.
	
	// reachability analysis (this is used in both reachability_status and build_AGT)
	int reachability_status; // 0:initial 1:being_traversed 2:there_is_path 3:no_path
	unsigned long time_last_tainted; // first time the node gets tainted!
	
	//cluster *clust; // used for AGT: shows of which seccon this node is a member. not initiated until AGT is built.
};

graph_edge *trace_parser(char *argv[], std::vector<node *> &graph);

/*
 * 
 * name: search_graph
 * @param: (vector <node *> &g, string str, string type="-1")
 * @return: return NULL, if the node doesn't exist, return the pointer to the node if the node exist.
 * 
 */
node *search_graph(vector <node *> &g, string str, string type);

/*
 * 
 * name: search_graph_edge
 * @param (vector <graph_edge *> graph_edges, string str)
 * @return return NULL, if the node doesn't exist, return the pointer to the node if the node exist.
 * 
 */
graph_edge *search_graph_edges(vector <graph_edge *> graph_edges, string str);

void print_nodes(std::vector<node *> &graph);

//void print_edges(std::vector<node *> &edges);


#endif
