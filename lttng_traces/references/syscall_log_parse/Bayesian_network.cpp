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

#include "dlib/bayes_utils.h"
#include "dlib/graph_utils.h"
#include "dlib/graph.h"
#include "dlib/directed_graph.h"

using namespace dlib;
using namespace std;
using namespace bayes_node_utils;


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&   Bayesian Networks Stuff  &&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

bool Bayesian_network_comment = true;

// Description: creates a bayesian_network given the dependency graph.
directed_graph<bayes_node>::kernel_1a_c *Bayesian_network(std::vector <node *> &graph)
{
	// to see how long building bayesian_network takes.
	gettimeofday(&start, NULL);
		
	// setting graph nodes' IDs
	for (unsigned int i=0; i<graph.size(); i++)
		graph[i]->ID = i;
	
	// update the edges' frequencies.
	node_behavoir_estimator(graph, false);
	
	// creating bayesian network
	directed_graph<bayes_node>::kernel_1a_c *bn = new directed_graph<bayes_node>::kernel_1a_c();
	
	// setting number of nodes
	bn->set_number_of_nodes(graph.size());
	
	// creating edges in the bayesian network
	for (unsigned int i=0; i<graph.size(); i++)
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			bn->add_edge(graph[i]->ID, graph[i]->children[j]->dest_node->ID);
			//cout << graph[i]->name << " -> " << graph[i]->children[j]->dest_node->name << " " << graph[i]->children[j]->frequency << endl;
		}
	
	// declaring that values are binary.
	for (unsigned int i=0; i<graph.size(); i++)
		set_node_num_values(*bn, i, 2);
	
	// assigning parents and CPTs.
	assignment parent_state;
	for (unsigned int i=0; i<graph.size(); i++){
		
		cout << "creating Bayesian node: " << graph[i]->name << " " << i << "/" << graph.size() << " #parents:" << graph[i]->parents.size() << endl;
		
		// the following way was wrong. because in case CPT_entries_num was '0', there was no value set by 'set_node_probability'
		//unsigned int CPT_entries_num = (graph[i]->parents.size() == 0) ? 0 : pow(2, graph[i]->parents.size());
		
		unsigned int CPT_entries_num = pow(2, graph[i]->parents.size());
		if (Bayesian_network_comment) cout << "\ncreation for " << graph[i]->name << " CPT_num:" << CPT_entries_num << endl;
		for (unsigned int j=0; j<CPT_entries_num; j++){
			
			parent_state.clear();
			int tmp = j;
			float product = 1; // probability that none of the parents write to the node.
			
			if (Bayesian_network_comment) cout << "\t";
			for (unsigned int k=0; k<graph[i]->parents.size(); k++){
				
				parent_state.add(graph[i]->parents[k]->src_node->ID, tmp%2);
				if (Bayesian_network_comment) cout << graph[i]->parents[k]->src_node->name << " (" << tmp%2 << ") ";
				
				// Gibbs sampler does not work with deterministic, i.e., 1, freqs
				float parent_freq; 
				if (fabs(graph[i]->parents[k]->src_frequency - 1) > 0.001)
					parent_freq = graph[i]->parents[k]->src_frequency;
				else
					parent_freq = .98;
				
				if (tmp%2 == 1)
					product *= (1 - parent_freq);
				tmp /= 2;
			}
			
			float CPT_entry = 1 - product; 
			set_node_probability(*bn, graph[i]->ID, 1, parent_state, CPT_entry);
			set_node_probability(*bn, graph[i]->ID, 0, parent_state, 1 - CPT_entry);
			
			if (Bayesian_network_comment) cout << ": " << CPT_entry << endl;
		}
	}
	
	// to see how long building bayesian_network takes.
	gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;	
	cout << "Constructing Bayesian took " << mtime << " msecs." << endl;
	
	return bn;
}

// this function needs revision!
void Bayesian_Inference(directed_graph<bayes_node>::kernel_1a_c *bn, std::vector <node *> &graph, unsigned int rounds){

	// to see how long bayesian inference takes.
	gettimeofday(&start, NULL);
	
	// to use it we should set the network to some initial state.
	for (unsigned int i=0; i<graph.size(); i++){
		
		if (graph[i]->evidence){
			
			graph[i]->probability = 1;
			set_node_value(*bn, graph[i]->ID, 1);
			set_node_as_evidence(*bn, graph[i]->ID);
			
		}else{
			graph[i]->probability = 0;
			set_node_value(*bn, graph[i]->ID, 0);
		}
	}
	
	// First create an instance of the gibbs sampler object
	bayesian_network_gibbs_sampler sampler;
	
	int counters[graph.size()]; // counters' indices are graph's indices (not graph nodes' IDs).
	for (unsigned int i=0; i<graph.size(); i++)
		counters[i] = 0;
	
	ofstream of1("2-time_required.dat");
    
    for (unsigned int i = 0; i<rounds; ++i){
		
		if (i%1000 == 0)
			cout << " round:" << i << endl;
        
		sampler.sample_graph(*bn);
		
		for (unsigned int j=0; j<graph.size(); j++)	
			if (node_value(*bn, graph[j]->ID) == 1)
				counters[j]++;
		
		
		float dif = 0;
		for (unsigned int j=0; j<graph.size(); j++){
			dif += fabs(graph[j]->probability - (counters[j]+0.0)/rounds);
			graph[j]->probability = (counters[j]+0.0)/i;
		}
		of1 << i << "\t" << dif << endl;
    }
    
	of1.close();
    
    for (unsigned int i=0; i<graph.size(); i++)
		graph[i]->probability = (counters[i]+0.0)/rounds;		

	// to see how long bayesian inference takes.
	gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;	
	cout << "Bayesian inference took " << mtime << " msecs." << endl;
	
}
