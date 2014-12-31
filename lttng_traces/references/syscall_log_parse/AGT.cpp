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


class node;

class cluster{ // used for CDG (clustered dependency graph)
public:
	string seccon; // security context
	std::vector <node *> nodes; // nodes in graph with the type 'seccon_type'
	
	cluster(string st){seccon = st;};
	cluster(){};
};


class AGT_edge;

class AGT_node{
public:
	int ID; // this is not set initially.
	string name;
	std::set <string> seccons; // security contexts (privilege domains owned by the attacker in this state)
	std::vector <AGT_edge *> children; // children of the node in directed graph
	std::vector <AGT_edge *> parents; // parents of the node in directed graph
	AGT_node(){ name = "default_name"; };
	AGT_node(string n){ name = n; };
};


class AGT_edge{
public:
	AGT_node *dest_node, *src_node; // source and destination nodes of the edge in the AGT.
	string vulnerability; // the vulnerability which was possibly exploited to escalate the privilege.
	node *process; // the process (node in the dependency graph) which contains the vulnerability.
	bool attack_vector_edge; // this is used for forensics (if the edge is a part of most_likely_edge?)
	
	AGT_edge(){
		process = NULL;
		attack_vector_edge = false;
	};
};

class AGT{
public:
	std::vector <AGT_node *> nodes;
	
	// the IDSes deployed to monitor the vulnerability. for each IDS-process, list of detected vuls.
	std::vector <pair <string[2], std::vector <string> > > already_deployed_idses; 
	AGT(){};
};





// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

string privs(AGT_node *);

void build_AGT(std::vector <node *> &graph, node *detection_point /* process causing the detection point */, 
				AGT *agt/* AGT is getting filled while the function recurses!*/, 
				VulDB *vuldb /* input: the VulDB which stores important info regarding IDS capabilities, etc. */,
				node *g = NULL /* recursion: current graph node */, 
				//AGT_node *a = NULL /* recursion: current AGT node*/,
				unsigned long current_time = 0 /* recursion: current time to do a time-sensitive traversal */){
	
	if (agt->nodes.size() == 0){ // initial call
		
		// sort times vector in edges. because AGT's construction below is time sensitive.
		sort_times_on_graph_edges(graph);
		
		// this is used for in DFS-like dependency graph's traversal
		// to make sure we do not loop while traversing the graph
		for (unsigned int i=0; i<graph.size(); i++)
			graph[i]->reachability_status = 0; 
		
		// setting initial AGT state (no privilege)
		AGT_node *initial_state = new AGT_node("initial_state"); // attacker has no privilege domain initially.
		agt->nodes.push_back(initial_state);
		//a = initial_state;
		
		// setting final sate (goal_state)
		// All the states with the required privilege level (of the process causing detection point)
		// are connected to this state without exploiting any vulnerability.
		AGT_node *goal_state = new AGT_node("goal_state"); 
		agt->nodes.push_back(goal_state);
		
		// setting initial node (attack's entry point) in the dependency graph (the socket)
		for (unsigned int i=0; i<graph.size(); i++){
			if (graph[i]->type == SOCK_AF_INET){
				
				if (g != NULL)
					cout << "Error:: < build_AGT >: there are more than one socket \
							in the graph! (this is not supported yet)." << endl;
				g = graph[i];
				g->reachability_status = 1;
			}
		}
		
		build_AGT(graph, detection_point, agt, vuldb, g, current_time);
		g->reachability_status = 0;
		// AGT-construction is over here. 
		
		// right before the final return.
		return;
	}
	
	// if the current node includes the necessary privilege domain,
	// create/trace the attack path in AGT and connect the final node to the 
	// goal_state and return (no need to go further because
	// we are only interested in knowing how the attacker got to the privilege).	
	
	if (g->seccon == detection_point->seccon){
		
		// searching for the goal and inital state in AGT
		AGT_node *agt_goal_state = NULL, *agt_initial_state = NULL;
		for (unsigned int i=0; i<agt->nodes.size(); i++){
			
			if (agt->nodes[i]->name == "goal_state") agt_goal_state = agt->nodes[i];
			if (agt->nodes[i]->name == "initial_state") agt_initial_state = agt->nodes[i];
			if (agt_initial_state && agt_goal_state) break;
		}
		
		// searching for entry_point (socket) in the dependency graph
		node *dep_entry_point = NULL;
		for (unsigned int i=0; i<graph.size(); i++){
			if (graph[i]->type == SOCK_AF_INET){
				dep_entry_point = graph[i];
				break;
			}
		}
		
		if (!agt_initial_state || !agt_goal_state || !dep_entry_point){
			cout << " Error:: < build_AGT > : some nodes could not in found in either agt or dep graph." << endl;
			exit(1);
		}
		
		// now we trace nodes and their privs in the dependency graph as marked incrementally in 
		// the recursive traversal and build/trace the attack graph template.		
		node *temp_dep_node = dep_entry_point;
		AGT_node *temp_agt_node = agt_initial_state;
		bool loop;
		
		do{
			
			loop = false;
			for (unsigned int i=0; i<temp_dep_node->children.size(); i++){
				if (temp_dep_node->children[i]->dest_node->reachability_status == temp_dep_node->reachability_status + 1){ // being traversed
					
					// is the privilege domain not gained yet?
					if (temp_agt_node->seccons.find(temp_dep_node->children[i]->dest_node->seccon) 
						== temp_agt_node->seccons.end()){
						
						std::set <string> security_contexts = temp_agt_node->seccons;
						security_contexts.insert(temp_dep_node->children[i]->dest_node->seccon);
						
						// is next_agt_state already in AGT?
						AGT_node *next_agt_state = NULL;
						for (unsigned int j=0; j<agt->nodes.size(); j++){
							if (agt->nodes[j]->seccons  ==  security_contexts){
								next_agt_state = agt->nodes[j];
								break;
							}
						}
						
						// nop! create the node.
						if (!next_agt_state){
							
							next_agt_state = new AGT_node();
							next_agt_state->seccons = security_contexts;
							agt->nodes.push_back(next_agt_state);
						}
						
						// connect current AGT node, i.e., temp_agt_node, to the next_agt_state. 
						for (unsigned int j=0; j<vuldb->process_vul[temp_dep_node->children[i]->dest_node->name].size(); j++){
						
							AGT_edge *e = new AGT_edge();
							e->src_node = temp_agt_node;
							e->dest_node = next_agt_state;
										
							e->vulnerability = vuldb->process_vul[temp_dep_node->children[i]->dest_node->name][j];
							e->process = temp_dep_node->children[i]->dest_node;
							
							temp_agt_node->children.push_back(e);
							next_agt_state->parents.push_back(e);
						}
						
						// for visual debugging purposes: will construct one edge if there is no vulnerability 
						// listed for the process in VulDB.
						if (vuldb->process_vul[temp_dep_node->children[i]->dest_node->name].size() == 0){
							
							AGT_edge *e = new AGT_edge();
							e->src_node = temp_agt_node;
							e->dest_node = next_agt_state;
										
							e->vulnerability = "no-vul-listed";
							e->process = temp_dep_node->children[i]->dest_node;
							
							temp_agt_node->children.push_back(e);
							next_agt_state->parents.push_back(e);
						}
						
						// preping the next while iteration.
						temp_agt_node = next_agt_state;
					}
					
					// preping the next while iteration.
					temp_dep_node = temp_dep_node->children[i]->dest_node;
					
					if (loop)
						cout << "Error:: < build_AGT >: there seems to be a loop in DFS!" << endl;
					loop = true;
					break;
				}
			}
		
		}while(loop);
		
		// now that the path in AGT is constructed, connect the last 
		// agt_node (which includes the required privilege domain for the detection point)
		// to the goal state in AGT.
		
		bool already_connected = false;
		for (unsigned int i=0; i<temp_agt_node->children.size(); i++)
			if (temp_agt_node->children[i]->dest_node == agt_goal_state)
				already_connected = true;
		
		if (!already_connected){
			
			AGT_edge *e = new AGT_edge();
			e->src_node = temp_agt_node;
			e->dest_node = agt_goal_state;
			e->vulnerability = "NOP";
			e->process = NULL;
			
			//e->exploits.push_back(*(new pair <node *, string> (NULL, "NOP")));
			
			temp_agt_node->children.push_back(e);
			agt_goal_state->parents.push_back(e);
		}
		
		return;
	}
	
	
	// the required privilege level is not obtained yet. traverse more.
	for (unsigned int i=0; i<g->children.size(); i++){
		
		// test
		for (unsigned int ti=1; ti<g->children[i]->times.size(); ti++){
			if (g->children[i]->times[ti] < g->children[i]->times[ti-1]){
				cout << "Error:: < build_AGT >: times vector is not sorted!" << endl;
				cout << g->name << "->" << g->children[i]->dest_node->name << " " << g->children[i]->times.size() << endl;
			}
		}
		
		//cout << "Saman:: build_AGT:: 3 child_name: " << g->children[i]->dest_node->name << " times.size():" << g->children[i]->times.size() << endl;
			
		// is it possible to go forward based on the
		// recursion:current_time and the times on the edge?
		int new_time = -1; 
		for (unsigned int ti=0; ti<g->children[i]->times.size(); ti++){
			
			if (current_time < g->children[i]->times[ti]){
				new_time = g->children[i]->times[ti];
				break;
			}
		}
		
		if (new_time != -1  &&  g->children[i]->dest_node->reachability_status == 0){ // yap! it's possible.
			
			node *gchild = g->children[i]->dest_node;
			
			gchild->reachability_status = g->reachability_status + 1; // we the mark the node 1 to avoid looping while DFSing.
			build_AGT(graph, detection_point, agt, vuldb, gchild, new_time);
			gchild->reachability_status = 0;
		}
	}
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

// Description: reachability_analysis does a backward taint tracking, via reachability analysis, 
// from the detection point to an INET socket node. 
// Finally, it removes all irrelevant nodes, which are not on the path from a socket to the detection point.
void reachability_analysis(std::vector <node *> &graph, pair <graph_edge *, int> &detection_point, 
							node *s_node = NULL /*node to trace back from*/,
							bool update_times = true)
{
	if (!s_node){ // initial call!
		
		// to measure time needed by the function (can be removed)
		gettimeofday(&start, NULL);
		
		// sort times vector in edges.
		sort_times_on_graph_edges(graph);
		
		for (unsigned int i=0; i<graph.size(); i++){
			
			if (graph[i]->type == SOCK_AF_INET) // sockets are the main targets, so initially mark them as reachable.
				 graph[i]->reachability_status = 2; 
			else graph[i]->reachability_status = 0; 
			
			graph[i]->time_last_tainted = 0;
		}
		
		// preping for next call
		if (detection_point.first->src_node->type == PROCESS) // getting the subject of the detection point (read or write)
			 s_node = detection_point.first->src_node;
		else s_node = detection_point.first->dest_node;
		
		s_node->reachability_status = 1;
		s_node->time_last_tainted = detection_point.first->times[detection_point.second];
		
		// update the times, i.e., time_last_tainted, via backward-recursion.
		reachability_analysis(graph, detection_point, s_node, true);
		
		// do the main time-sensitive reachability analysis.
		reachability_analysis(graph, detection_point, s_node, false);
		
		// analysis is over here. refine the graph.
		for (unsigned int i=0; i<graph.size(); i++){
			
			if (graph[i]->reachability_status != 2){
				remove_graph_node(graph, i);
				i = 0;
			}
		}
		
		// to see how long building bayesian_network takes.
		gettimeofday(&end, NULL);
		seconds  = end.tv_sec  - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;
		mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;	
		cout << "Reachability analysis took " << mtime << " msecs." << endl;	
		
		// right before final return.
		return;
	}
	
	// 1st recursive sub-function.
	// backward-recursion: update times, i.e., time_last_tainted's.
	if (update_times){
		
		for (unsigned int i=0; i<s_node->parents.size(); i++){
			
			bool updated = false;
			
			//cout << "\n\nMarina 0\t" << s_node->parents[i]->src_node->name << " " << s_node->parents[i]->times.size() << endl;
			
			for (int j=s_node->parents[i]->times.size()-1; 0<=j; j--){
				
				//cout << "j(" << j << "): " << s_node->parents[i]->times[j] << " < " << s_node->time_last_tainted << "  &&  " << s_node->parents[i]->src_node->time_last_tainted << " < " << s_node->parents[i]->times[j]<< endl;getchar();
				
				if (s_node->parents[i]->times[j] < s_node->time_last_tainted
					&&  s_node->parents[i]->src_node->time_last_tainted < s_node->parents[i]->times[j]){
					
					s_node->parents[i]->src_node->time_last_tainted = s_node->parents[i]->times[j];;
					
					updated = true;
					break;
				}
			}
			
			if (updated) 
				reachability_analysis(graph, detection_point, s_node->parents[i]->src_node, update_times);
		}
	
	// 2nd recursive sub-function.
	// Recursively traceback from s_node.
	}else{
		
		for (unsigned int i=0; i<s_node->parents.size(); i++){
			
			int status = s_node->parents[i]->src_node->reachability_status;
			
			// if it has its initial value, we go through if it is timewise possible.
			if (status == 0  &&  s_node->parents[i]->src_node->time_last_tainted <= s_node->time_last_tainted){ 
				s_node->parents[i]->src_node->reachability_status = 1;
				reachability_analysis(graph, detection_point, s_node->parents[i]->src_node, update_times);
				if (s_node->parents[i]->src_node->reachability_status != 2) // if not already resolved, there is no path.
					s_node->parents[i]->src_node->reachability_status = 3;
				
			}else if (status == 2){ // reachable! mark all the being_traverseds behind as reachable.
				for (unsigned int j=0; j<graph.size(); j++)
					if (graph[j]->reachability_status == 1)
						graph[j]->reachability_status = 2;
			}
		}
	}
	
	return;
}

