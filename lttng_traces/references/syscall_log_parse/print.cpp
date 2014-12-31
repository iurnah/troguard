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



// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

string privs(AGT_node *agt){
	
	string str = "";
	
	std::set <string>::iterator it; 
	for (it = agt->seccons.begin(); it != agt->seccons.end(); ++it)
		str = str + *it + " ";
	
	if (str != "")
		str = ":" + str;
	
	return str;
}

void AGT2dot(VulDB *vuldb, AGT *agt){
	
	ofstream of1("./agt.dot");
	of1 << "digraph G {" << endl;

	for (unsigned int i=0; i<agt->nodes.size(); i++){
		for (unsigned int j=0; j<agt->nodes[i]->children.size(); j++){
			of1 << "\t" << "\"" << agt->nodes[i]->name << privs(agt->nodes[i]) << "\"" << " -> " << "\"" 
				<< agt->nodes[i]->children[j]->dest_node->name << privs(agt->nodes[i]->children[j]->dest_node) << "\"";
				
				if (agt->nodes[i]->children[j]->attack_vector_edge  ||  agt->nodes[i]->children[j]->process){
					
					of1 << " [ ";
					
					if (agt->nodes[i]->children[j]->attack_vector_edge)
						of1 << "style=bold";
					
					if (agt->nodes[i]->children[j]->attack_vector_edge  &&  agt->nodes[i]->children[j]->process)
						of1 << ", ";
					
					if (agt->nodes[i]->children[j]->process)
						of1 << "label=" << "\"" << agt->nodes[i]->children[j]->process->name << ":" << agt->nodes[i]->children[j]->vulnerability << "(" << weight(vuldb, agt, agt->nodes[i]->children[j]) << ")\"" << endl;
						
					of1 << " ]";
				}
				
				of1 << " ;";
		}
	}
	
	of1 << "\n}" << endl;
	of1.close();
	
	system("dot -Tpdf agt.dot -o agt.pdf; acroread agt.pdf &");
	return;
}

// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

string node_name_log(node *n, bool with_type){
	if (with_type)
		return ("\"" + n->type + ":" + n->name + "\"");
	else
		return ("\"" + n->name + "\"");
}

// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

void graph2dot(std::vector <node *> &graph, bool with_type=true){
	
	ofstream of1("./graph.dot");
	of1 << "digraph G {" << endl;

	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			of1 << "\t" << node_name_log(graph[i], with_type) << " -> " << node_name_log(graph[i]->children[j]->dest_node, with_type)
				<< "[label=" << graph[i]->children[j]->times.size() << "] " << ";" << endl;
		}
	}
	
	of1 << "}" << endl;
	of1.close();
	
	system("dot -Tpdf graph.dot -o graph.pdf; acroread graph.pdf &");
	return;
}

// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

void graph2CDG2dot(std::vector <node *> &graph, bool with_type=true){ // clustered dependency graph
	
	// initializing tainted vars (these will be used when printing clusters).
	for (unsigned int i=0; i<graph.size(); i++)
		graph[i]->reachability_status = 0;
	
	// clustering the nodes in dep graph
	std::vector <cluster *> CDG;
	for (unsigned int i=0; i<graph.size(); i++){
		
		string search_str = (graph[i]->type == PROCESS) ? graph[i]->seccon : "object";
		
		cluster *c = NULL;
		for (unsigned int j=0; j<CDG.size(); j++)
			if (CDG[j]->seccon == search_str)
				c = CDG[j];
		
		if (!c){
			c = new cluster(search_str);
			CDG.push_back(c);
		}
		
		c->nodes.push_back(graph[i]);
		graph[i]->clust = c; 
	}
	
	// writing to dot file.
	ofstream of1("./cdg.dot"); 
	of1 << "digraph G {" << endl << "\tsize=\"8,6\"; ratio=fill; node[fontsize=24];" << endl;
	
	// drawing edges
	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			
			//if (graph[i]->clust->seccon != graph[i]->children[j]->dest_node->clust->seccon)

				of1 << "\t" << node_name_log(graph[i], with_type) << " -> " << node_name_log(graph[i]->children[j]->dest_node, with_type) 
					<< " [label=" << graph[i]->children[j]->times.size() << "] " << ";" << endl;
					
				graph[i]->reachability_status = 1;
				graph[i]->children[j]->dest_node->reachability_status = 1;
			
		}
	}
	
	// introducing clusters
	of1 << endl;
	for (unsigned int i=0; i<CDG.size(); i++){
		of1 << "\tsubgraph \"cluster_" << CDG[i]->seccon << "\" {" << endl 
			<< "\t\tlabel=\"" << CDG[i]->seccon << "\";\n";
		for (unsigned int j=0; j<CDG[i]->nodes.size(); j++){
			if (CDG[i]->nodes[j]->reachability_status == 1)
				of1 << "\t\t" << node_name_log(CDG[i]->nodes[j], with_type) << " ;" << endl;			
		}
		of1 << "\t}" << endl;
	}
	
	
	of1 << "}" << endl;
	of1.close();
	
	system("dot -Tpdf cdg.dot -o cdg.pdf; acroread cdg.pdf &");
	return;
}


void node2dist(std::vector <node *> &graph, string node_name){
	
	int index = -1;
	for (unsigned int i=0; i<graph.size(); i++){
		
		if (graph[i]->name == node_name)
			index = i;
	}
	
	if (index == -1){
		cout << " Error<node2dist>:: node " << node_name << " was not found in the graph!" << endl;
		return;
	}
	
	// updating edges' frequencies of all nodes
	node_behavoir_estimator(graph);
	
	// writing to the file
	string file_name = "./"+node_name+"_behavior.xgraph";
	ofstream of(file_name.c_str());
	if (!of){
		cout << "Error:: < node2dist >:: file " << file_name << " could not be opened!" << endl;
		return;
	}
	
	for (unsigned int i=0; i<graph[index]->parents.size(); i++) 
		of << "Read-from-" << graph[index]->parents[i]->src_node->name 
			<< "\t" << graph[index]->parents[i]->dest_frequency << endl;
	
	for (unsigned int i=0; i<graph[index]->children.size(); i++) 
		of << "Write-to-" << graph[index]->children[i]->dest_node->name 
			<< "\t" << graph[index]->children[i]->src_frequency << endl;
	
	of.close();
}

