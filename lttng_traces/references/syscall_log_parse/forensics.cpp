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

// Description: calculates the weight of the edge based on 
// the already_deployed_idses and their results.
float weight(VulDB *vuldb, AGT *agt, AGT_edge *agt_edge){
	
	//cout << "weight: edge from <" << privs(agt_edge->src_node) << "> to <" << privs(agt_edge->dest_node) << ">" << endl;
	
	float weight = 1;
	for (unsigned int i=0; i<agt->already_deployed_idses.size(); i++){
		
		string ids_name = agt->already_deployed_idses[i].first[0];
		string process = agt->already_deployed_idses[i].first[1];
		
		// if the process under consideration is not the process being monitored, ignore.
		if (!agt_edge->process  ||  agt_edge->process->name != process)
			continue;
		
		float fp_rate, fn_rate;
		
		bool ids_result = true;
		if (find(agt->already_deployed_idses[i].second.begin(), agt->already_deployed_idses[i].second.end(), agt_edge->vulnerability) == agt->already_deployed_idses[i].second.end())
			ids_result = false;
				
		bool found = false;
		for (unsigned int j=0; j<vuldb->vul_ids[agt_edge->vulnerability].size(); j++){
			
			if (vuldb->vul_ids[agt_edge->vulnerability][j]->ids_name == ids_name){
				fp_rate = vuldb->vul_ids[agt_edge->vulnerability][j]->fp_rate;
				fn_rate = vuldb->vul_ids[agt_edge->vulnerability][j]->fn_rate;
				found = true;
				break;
			}
		}
		
		if (found)
			weight *= ids_result ? (1 - fp_rate) : fn_rate;
	}
	
	//cout << "end of weight." << endl;
	
	return weight;
}

// Description: determines if the idsProcess combination has been deployed.
bool is_already_deployed(AGT *agt, string ids, string process){
	
	for (unsigned int i=0; i<agt->already_deployed_idses.size(); i++)
		if (agt->already_deployed_idses[i].first[0] == ids  &&  agt->already_deployed_idses[i].first[1] == process)
			return true;
	return false;
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

// Description: determines the best next IDS to turn on 
// according the AGT and already_deployed_idses.
pair<string,string> best_ids(VulDB *vuldb, AGT *agt){
	
	float best_idsProcess_score = -1;
	pair <string, string> best_ids_process("no-ids-selected", "no-process-selected"); // (ids, process)
	
	// iterations to estimate the strenght of each (ids,process) pair.
	for (unsigned int i=0; i<vuldb->idses.size(); i++){
		for (unsigned int ii=0; ii<vuldb->processes.size(); ii++){
			
			if (is_already_deployed(agt, vuldb->idses[i].first, vuldb->processes[ii]))
				continue;
			
			//cout << "\ntesting IDS:: " << vuldb->idses[i] << "-" << vuldb->processes[ii] << endl;
			
			float current_idsProcess_coverage = 0;
			for (unsigned int j=0; j<agt->nodes.size(); j++){ // to calculate the ids_score of the current ids, all the AGT_edges are investigated.
				
				for (unsigned int k=0; k<agt->nodes[j]->children.size(); k++){					
					
					for (unsigned int l=0; l<vuldb->vul_ids[agt->nodes[j]->children[k]->vulnerability].size(); l++){
						
						// if the current ids can detect the current exploitation
						if (vuldb->vul_ids[agt->nodes[j]->children[k]->vulnerability][l]->ids_name == vuldb->idses[i].first
							&& agt->nodes[j]->children[k]->process->name == vuldb->processes[ii]){
							
							// to estimate the strength of a ids, we assume the positive/negative prior dist is known.
							float prior_dist_pn[2] = {0.5, 0.5}; // prior prob of (not) having the exploitation
							
							string idsProcess[2] = {vuldb->idses[i].first, vuldb->processes[ii]};
							
							float w0, w_true, w_false;
							w0 = weight(vuldb, agt, agt->nodes[j]->children[k]);
							
							agt->already_deployed_idses.push_back(pair<string[2], bool>(idsProcess, true));
							w_true = weight(vuldb, agt, agt->nodes[j]->children[k]);
							agt->already_deployed_idses.pop_back();
							
							agt->already_deployed_idses.push_back(pair<string[2], bool>(idsProcess, false));
							w_false = weight(vuldb, agt, agt->nodes[j]->children[k]);
							agt->already_deployed_idses.pop_back();
							
							// weight improvement if the current ids is used.
							current_idsProcess_coverage += prior_dist_pn[0]*fabs(w0 - w_true) + prior_dist_pn[1]*fabs(w0 - w_false);
						}
					}
				}
			}
			
			float current_idsProcess_score = current_idsProcess_coverage/vuldb->idses[i].second; // score = coverage/cost.
			
			if (best_idsProcess_score < current_idsProcess_score  &&  0.0001<fabs(current_idsProcess_score)){
				
				best_ids_process.first = vuldb->idses[i].first;
				best_ids_process.second = vuldb->processes[ii];
				best_idsProcess_score = current_idsProcess_score;
			}
		}
	}
	
	return best_ids_process;
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

// Description: most_likely_path finds the path from the initial_state to the goal_state in AGT
// given the edge weights from the forensics analysis phase.
std::vector <AGT_edge *> most_likely_path(VulDB *vuldb, AGT *agt, AGT_node *agt_node = NULL){
	
	if (agt_node == NULL){ // the base (initial) call.
	
		// finding the initial and the goal states in agt.
		AGT_node *initial_state = NULL, *goal_state = NULL;
		for (unsigned int i=0; i<agt->nodes.size(); i++){
			
			if (agt->nodes[i]->name == "initial_state")
				initial_state = agt->nodes[i];
			if (agt->nodes[i]->name == "goal_state")
				goal_state = agt->nodes[i];
		}
		
		if (!initial_state || !goal_state){
			cout << "Error:: <automated_forensics> : Could not find the initial/goal state in AGT." << endl;
			exit(1);
		}
		
		//cout << "in base call just before return!" << endl;
		std::vector <AGT_edge *> path = most_likely_path(vuldb, agt, initial_state);
		
		// setting the attack_vector_edge variable in the attacked edges.
		for (unsigned int i=0; i<path.size(); i++)
			path[i]->attack_vector_edge = true;
		
		// just before the last return.
		return path;
	}
	
	// the last call (agt_node is the goal_state of the agt)
	if (agt_node->name == "goal_state"){
		std::vector <AGT_edge *> path;
		return path;
	}
	
	// not the base call. recursively find the best path.
	int best_path_index = -1;
	double best_path_probability = -1, tmp_path_probability = -1;
	for (unsigned int i=0; i<agt_node->children.size(); i++){
		
		//cout << "before weight i:" << i << " agt_node:" << privs(agt_node) <<  endl;
		tmp_path_probability = weight(vuldb, agt, agt_node->children[i]);
		//cout << "after weight." << endl;
		
		std::vector <AGT_edge *> path = most_likely_path(vuldb, agt, agt_node->children[i]->dest_node);
		for (unsigned int j=0; j<path.size(); j++)
			tmp_path_probability *= weight(vuldb, agt, path[j]);
		
		if (best_path_probability < tmp_path_probability){
			best_path_index = i;
			best_path_probability = tmp_path_probability;
		}
	}
	
	//cout << "best_index:" << best_path_index << " best_path_probability: " << best_path_probability << endl;
	
	// best path is constructed and returned.
	std::vector <AGT_edge *> path = most_likely_path(vuldb, agt, agt_node->children[best_path_index]->dest_node);
	path.insert(path.begin(), agt_node->children[best_path_index]);
	//cout << "in function: " << path.size() << endl;
	return path;
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

// Description: to do automated forensics based on the generated AGT,
// and already deployed idses and their results.
void automated_forensics(VulDB *vuldb, AGT *agt){
	
	cout << "\n\nStarting Forensics Analysis..." << endl;
	
	do{
		
		pair <string, string> next_ids = best_ids(vuldb, agt);
		
		if (next_ids.first == "no-ids-selected") // no useful IDS could be selected.
			break;
		
		string ids[2] = {next_ids.first, next_ids.second};
		std::vector <string> detected_vuls;
		
		cout << "\n\t" << next_ids.first << ":" << next_ids.second << endl;
		cout << "\tEnter -1 to stop the forensics." << endl;
		cout << "\tResult (";
		for (unsigned int i=0; i<vuldb->ids_vul[next_ids.first].size(); i++)
			cout << i << ":" << vuldb->ids_vul[next_ids.first][i] << " ";
		cout << "): ";
		
		string result_str, tmp_str;
		
		char result_chr[512];
		cin.getline(result_chr, 512);
		
		if (atoi(result_chr) == -1) break;
		
		result_str.insert(0, result_chr, strlen(result_chr));
		
		istringstream strm(result_str);
		
		while (!strm.eof()){
			strm >> tmp_str;
			detected_vuls.push_back(vuldb->ids_vul[next_ids.first][atoi(tmp_str.c_str())]);
		}
		
		agt->already_deployed_idses.push_back(pair <string[2], std::vector<string> > (ids, detected_vuls));
		
	} while (true);
	
	// get the most likely path.
	most_likely_path(vuldb, agt);
	
}
