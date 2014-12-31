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

using namespace std;
using std::vector;


class node;
class cluster;

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

class node{ // used as nodes in dependency graph
public:
	int ID; // this is not set initially. (currently, used in bayesian_network creation.)
	
	string name, type;
	
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
	
	cluster *clust; // used for AGT: shows of which seccon this node is a member. not initiated until AGT is built.
};

// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


bool TimesSortPredicate(long unsigned t1, long unsigned t2){
  return t1 < t2;
}

void sort_times_on_graph_edges(vector <node *> &graph){
	
	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++)
			sort(graph[i]->children[j]->times.begin(), graph[i]->children[j]->times.end(), TimesSortPredicate);
		for (unsigned int j=0; j<graph[i]->parents.size(); j++)
			sort(graph[i]->parents[j]->times.begin(), graph[i]->parents[j]->times.end(), TimesSortPredicate);
	}
}



graph_edge *search_graph_edges(vector <graph_edge *> graph_edges, string str){
	for (unsigned int i=0; i<graph_edges.size(); i++)
		if (graph_edges[i]->dest_node->name == str)
			return graph_edges[i];
	return NULL;
}


node *search_graph(vector <node *> &g, string str, string type="-1"){ // reminder: e.g. type=PROCESS
	for (unsigned int i=0; i<g.size(); i++)
		if (g[i]->name == str  &&  ( type=="-1" || g[i]->type==type ))
			return g[i];
	return NULL;
}


void remove_graph_node(vector <node *> &graph, unsigned int index)
{
	for (unsigned int i=0; i<graph.size(); i++){
		
		if ( i == index)
			continue;
		
		// investigating children
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			
			if (graph[i]->children[j]->dest_node == graph[index]){
				graph[i]->children.erase( graph[i]->children.begin() + j );
				j=0;
			}
		}
		
		// investigating parents
		for (unsigned int j=0; j<graph[i]->parents.size(); j++){
			
			if (graph[i]->parents[j]->src_node == graph[index]){
				graph[i]->parents.erase( graph[i]->parents.begin() + j );
				j=0;
			}
		}
	}
	
	graph.erase(graph.begin() + index);
	return;
}



bool is_isolated_node(vector <node *> &graph, node *nod){
	
	if (nod->children.size() != 0)
		return false;
	
	for (unsigned int i=0; i<graph.size(); i++)
		if (search_graph_edges(graph[i]->children, nod->name))
			return false;
	
	return true;
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

void socket_O_finder(vector <node *> &graph, node *graph_node, bool first_call = true){
	
	if (first_call)
		cout << "I::" << graph_node->name << endl;
	
	if (!first_call && graph_node->type == SOCK_AF_INET){
		cout << "\t" << graph_node->name << endl;
		return;
	}
	
	// mark the node as being traversed!
	graph_node->reachability_status = 1;
	
	// recursive search
	for (unsigned int i=0; i<graph_node->children.size(); i++){
		
		if (graph_node->children[i]->dest_node->reachability_status != 1)
			socket_O_finder(graph, graph_node->children[i]->dest_node, false);
	}

	// "un"-mark the node as being traversed!
	graph_node->reachability_status = 0;

}

void socket_IO_mapper(vector <node *> &graph){
		
	for (unsigned int i = 0; i<graph.size(); i++){
		
		// find the sockets
		if (graph[i]->type == SOCK_AF_INET){
			
			// first, clear the reachability bits
			for (unsigned int j = 0; i<graph.size(); j++)
				graph[j]->reachability_status = 0;
				
			// fo a data-flow DFS search from the socket to find a socket!
			socket_O_finder(graph, graph[i]);
			
		}
	}
	
	return;
}



// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

// Description: refine_graph removes all the pipes, unix sockets, and ipc_message_queues from the graph
// reachability_analysis MUST be called after refine_graph because removing pipes may result in isolated 
// processes (A's), which haved interacted only with self-pipes
void refine_graph(vector <node *> &graph, bool remove_files_aswell = true) // A(process) -> B(pipe) -> C(process)
{
	
	// to measure time needed by the function (can be removed)
	gettimeofday(&start, NULL);
		
	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			if (graph[i]->children[j]->dest_node == graph[i]){
				cout << "SELF-LOOP-c: " << graph[i]->name << endl;
			}
		}
		for (unsigned int j=0; j<graph[i]->parents.size(); j++){
			if (graph[i]->parents[j]->src_node == graph[i]){
				cout << "SELF-LOOP-p: " << graph[i]->name << endl;
			}
		}
	}
	
	for (unsigned int i=0; i<graph.size(); i++){
		
		if (graph[i]->type == FIFO  ||  graph[i]->type == SOCK_AF_UNIX  ||  graph[i]->type == IPCMSG 
			||  (graph[i]->type == REG  &&  remove_files_aswell)){ // B(pipe)
			
			for (unsigned int j=0; j<graph[i]->children.size(); j++){ // C's
				for (unsigned int k=0; k<graph[i]->parents.size(); k++){ // A's
					
					if (graph[i]->parents[k]->src_node != graph[i]->children[j]->dest_node){ // dest process is not the one which wrote to pipe!
						
						//cout << "LOG:: " << graph[i]->parents[k]->src_node->name << "->" << graph[i]->name 
							//<< "->" << graph[i]->children[j]->dest_node->name << endl;
						
						// need to find the edge (node, dest_node) if it already exists
						bool graph_edge_created_here = false; // if so, it will be added to the graph
						graph_edge *e = search_graph_edges(graph[i]->parents[k]->src_node->children, graph[i]->children[j]->dest_node->name);
						if (!e){
							e = new graph_edge(graph[i]->parents[k]->src_node, graph[i]->children[j]->dest_node);
							graph_edge_created_here = true;
						}
						
						bool log = true; // for logging purposes (could be removed)
						for (unsigned int l=0; l<graph[i]->children[j]->times.size(); l++){
							unsigned long min = *min_element(graph[i]->parents[k]->times.begin(), graph[i]->parents[k]->times.end());
							if (min <= graph[i]->children[j]->times[l]){
								
								e->times.push_back(graph[i]->children[j]->times[l]);
								if (log){
									log = false; 
									//cout << "src:" << src_ngbrIndex[k].first->name << "@" << min << " to " << graph[i]->name << " to " << graph[i]->neighbors[j]->dest_node->name << "@" << graph[i]->neighbors[j]->times[l] << endl;
								}
							}
						}
						
						if (graph_edge_created_here  &&  e->times.size() != 0){
							graph[i]->children[j]->dest_node->parents.push_back(e);
							graph[i]->parents[k]->src_node->children.push_back(e);
						}
					}
				}
			}
			
			// DONE with the pipe. remove the pipe and the edges to it!
			remove_graph_node(graph, i);
			i=0;
		}
	}
	
	// to see how long building bayesian_network takes.
	gettimeofday(&end, NULL);
	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;
	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;	
	cout << "Refining the graph took " << mtime << " msecs." << endl;	
}



// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

// updates frecuency of each edge in the graph. 
double node_behavoir_estimator(vector <node *> &graph, bool consider_reads = true){
	
	double updated_delta = 0;
	for (unsigned int i=0; i<graph.size(); i++){
		
		// updating in_stat and out_stat of all nodes
		float sum = 0;
		
		if (consider_reads)
			for (unsigned int j=0; j<graph[i]->parents.size(); j++) 
				sum += graph[i]->parents[j]->times.size();
		
		for (unsigned int j=0; j<graph[i]->children.size(); j++) 
			sum += graph[i]->children[j]->times.size();
		
		if (consider_reads)
			for (unsigned int j=0; j<graph[i]->parents.size(); j++){
				
				float new_value = (graph[i]->parents[j]->times.size()+0.0)/sum;
				updated_delta += fabs(new_value - graph[i]->parents[j]->dest_frequency);
				graph[i]->parents[j]->dest_frequency = new_value;
			}
		
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			
			float new_value = (graph[i]->children[j]->times.size()+0.0)/sum;
			updated_delta += fabs(new_value - graph[i]->children[j]->src_frequency);
			graph[i]->children[j]->src_frequency = new_value;
		}
	}
	return updated_delta;
}

// Description: set_evidence is used to set evidence nodes (argument: name) in the Bayesian network
// if pessimistic is true, it will also mark all the nodes within the same domain as tainted!
// if name is not provided, it will mark every node as false.
bool set_evidence(vector <node *> &graph, string name = "@CLEAR-EVERYTHING@", bool pessimistic = false){
	
	// if no name is provided, just clear every node.
	if (name == "@CLEAR-EVERYTHING@"){
		for (unsigned int i=0; i<graph.size(); i++)
			graph[i]->evidence = false;
		return true;
	}
	
	int index = -1;
	for (unsigned int i=0; i<graph.size(); i++){
		
		if (graph[i]->name == name){
			index = i;
			cout << "putting evidence: " << graph[i]->name << endl;
			graph[i]->evidence = true;
		}
	}
	
	if (index == -1)
		return false;
	
	if (!pessimistic)
		return true;
	
	for (unsigned int i=0; i<graph.size(); i++){
		
		if (graph[i]->seccon == graph[index]->seccon){
			cout << "putting evidence: " << graph[i]->name << endl;
			graph[i]->evidence = true;
		}
	}
	
	return true;
}
