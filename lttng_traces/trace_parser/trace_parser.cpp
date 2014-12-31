#include "common.h"
#include "trace_parser.h"

graph_edge *trace_parser(char *argv[], std::vector<node *> &graph){
	
	ifstream logfile(argv[1]);
	
	if(!logfile){
		cout << "Error: could not open the file!" << endl;
		exit(1);
	}
	
	struct timeval start, end;
	long mtime, seconds, useconds;
	unsigned long time = 0;
	
	string line;
	//define substring for hold different information 
	string tmp, syscall, process_comm, parent_proc, filename, 
		file_type, object_type, object_name, subject_type, 
		file_path, child_name; 
	//use parent_proc for subject_name,
	
	gettimeofday(&start, NULL);
	graph_edge *detection_point_graph_edge = NULL;
	int skipped_Records = 0;
	string comm_len ("comm = \"");
	
	while (!logfile.eof()){
		time++;
		
		int sub_obj_toggle = 0;
		// Read line by lines
		getline(logfile, line);
		if(line.find(APP) == string::npos || 
			line.find(EXIT_SYSCALL) != string::npos || 
			line.find(SYS_NEWSTAT) !=string::npos )
			//ignore the "exit_syscall" and 
			continue;
		
		//parsing the line		
		if(line.find("comm = \"firefox\"") == string::npos && 
			line.find(FILENAME) != string::npos && 
			line.find("sys_execve: ") != string::npos){
			// find the record has both "comm = <something else>" and
			// "filename =" filed. how the firefox process has been forked
			subject_type = PROCESS; //the process must be the one that fork firefox.
			file_path = line.substr(line.find(FILENAME)+FILENAME_LEN, 
					line.find_first_of("\"", line.find(FILENAME) + FILENAME_LEN)-(line.find(FILENAME) + FILENAME_LEN));
					
			process_comm = line.substr(line.find(comm_len)+comm_len.size(),
					line.find_first_of("\"", line.find(comm_len) + comm_len.size())-(line.find(comm_len) + comm_len.size()));
			//********ToDo**********
			//use getCommField() and getFilenameField() function 
			//to reduce the code volume		
			child_name = file_path.substr(file_path.rfind("/")+1);
			sub_obj_toggle = 1;
			//debug print 
			cout << "  #!FIREFOX PROCESS CREATED by PROCESS: " << process_comm << endl;
			cout << "  #!FIREFOX PROCESS CREATED (name): " << child_name << endl;
			cout << "  #!FIREFOX PROCESS CREATED (From Path): " << file_path << endl;	
			//cout << "*PROCESS CREATED: " << line.substr(line.find("sys_")) << endl;
			child_name = process_comm;
		}else if(line.find("comm = \"firefox\"") != string::npos && 
				line.find(FILENAME) != string::npos)
				//find the record has both "comm = "firefox"" and "filename =" filed
		{
			file_path = line.substr(line.find(FILENAME)+FILENAME_LEN, 
					line.find_first_of("\"", line.find(FILENAME) + FILENAME_LEN)-(line.find(FILENAME) + FILENAME_LEN));
			if(line.find("sys_execve: ") != string::npos){
				subject_type = PROCESS; //The process must be firefox
				//process_comm = line.substr(line.find(comm_len)+comm_len.size(),line.find_first_of("\"", line.find(comm_len) + comm_len.size())-(line.find(comm_len) + comm_len.size()));
				child_name = file_path.substr(file_path.rfind("/")+1);
				
				sub_obj_toggle = 1;
			    //debug print
				//cout << " %PROCESS CREATED by PARENT PROCESS: " << process_comm << endl;
				cout << " %PROCESS CREATED BY FIREFOX (name): " << child_name << endl;
				cout << " %PROCESS CREATED BY FIREFOX (From Path): " << file_path << endl;
				//cout << "*PROCESS CREATED: " << line.substr(line.find("sys_")) << endl;
			
			}else if(line.find(".so") != string::npos || line.find(".mo") != string::npos ){
				object_type = LIBRARY;
				process_comm = line.substr(line.find(comm_len)+comm_len.size(), 
						line.find_first_of("\"", line.find(comm_len) + comm_len.size())-(line.find(comm_len) + comm_len.size()));
				object_name = file_path.substr(file_path.rfind("/")+1);
			
			/* //debug print
				cout << "@LIBRARY ACCESSED BY " << process_comm << ": " << file_path << endl;
				cout << "@LIBRARY NAME: " << object_name << endl;
			*/
			}else if(line.find(".so") == string::npos){
				object_type = REG;
				process_comm = line.substr(line.find(comm_len)+comm_len.size(), 
						line.find_first_of("\"", line.find(comm_len) + comm_len.size())-(line.find(comm_len) + comm_len.size()));
				object_name = file_path.substr(file_path.rfind("/")+1);
			/* //debug print
				cout << "<<FILE ACCESSED BY " << process_comm << ": " << file_path << endl;
				cout << "<<FILE NAME: " << object_name << endl;
				//cout << line.substr(line.find("sys_")) << endl;
			*/
			}	
		}else if(line.find("firefox") != string::npos && line.find("IPv4") != string::npos){
				object_type = IPv4_SOCK;
				object_name = line.substr(line.find("IPv4")-5, 2) + ":" +line.substr(line.find("IPv4") + 5, 6);
				file_path = line.substr(line.find("->")+2);
		
				//cout << "Object_type: " << object_type << "  Object_name:(df:inode) " << object_name << "  domain:(proto) " << file_path << endl; 
		}else{
			skipped_Records++;
			continue;
		}
		//cout << "\tskipped_Records: " << skipped_Records++ << endl;
		
		node *subj = search_graph(graph, child_name, subject_type);
		node *obj = search_graph(graph, object_name, object_type);
		
		if(!subj){
			subj = new node(PROCESS, child_name);
			graph.push_back(subj);
			subj-> path = file_path;
		//may need add PID later; 
		}
	
		if(!obj){
			obj = new node(object_type, object_name);
			if(object_type == REG){
				obj->path = file_path;
			}else if(object_type == LIBRARY){
				obj->path = file_path;
			}else if(object_type ==IPv4_SOCK){
				obj->path = file_path;
			}
			graph.push_back(obj);
		}
		
		graph_edge *e;
		
		if(sub_obj_toggle == 1){
			e = search_graph_edges(subj->children, obj->name);
			if (!e){
				e = new graph_edge(subj, obj, time);
				subj->children.push_back(e);
				obj->parents.push_back(e);
			}else 
				e->times.push_back(time);				
		}else if (sub_obj_toggle == 0){
			e = search_graph_edges(obj->children, subj->name);
			if(!e){
				e = new graph_edge(obj, subj, time);
				obj->children.push_back(e);
				subj->parents.push_back(e);
			}else
				e->times.push_back(time);
		}
			
	/*
		s >> tmp >> syscall >> process_comm >> filename; 
		
		cout << tmp << "  " << syscall << " " << process_comm << " " << endl;
		
		cout << time << endl;
	*/
	}//while loop terminate

	
	logfile.close();
	/* //this is to check the total number of processed records;
	cout << "Useful record processed: \n\t" << count << endl;
	cout << "skipped record: \n\t" << skipped_Records << endl;
	cout << "Total sys_exit record: \n\t" << exit_count << endl;
	cout << "Total record porcessed: \n\t" << count + skipped_Records + exit_count << endl;
	*/
	gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;	
	
	cout << "\n#:: parsing the file (while loop) is Done: \n\t" 
		<< mtime << " msecs \n\t" << endl;
		//<<  unrecognized_objtype << " of " << time << " lines had unknown object types!" << endl;
	
	return detection_point_graph_edge;
}

/*
 * 
 * name: search_graph
 * @param: (vector <node *> &g, string str, string type="-1")
 * @return: return NULL, if the node doesn't exist, return the pointer to the node if the node exist.
 * 
 */
node *search_graph(vector <node *> &g, string str, string type="-1"){// reminder: e.g. type=PROCESS
	for (unsigned int i=0; i<g.size(); i++)
		if (g[i]->name == str  &&  ( type=="-1" || g[i]->type==type ))
			return g[i];
	return NULL;
}

/*
 * 
 * name: search_graph_edge
 * @param (vector <graph_edge *> graph_edges, string str)
 * @return return NULL, if the node doesn't exist, return the pointer to the node if the node exist.
 * 
 */
 graph_edge *search_graph_edges(vector <graph_edge *> graph_edges, string str){
	for (unsigned int i=0; i<graph_edges.size(); i++)
		if (graph_edges[i]->dest_node->name == str)
			return graph_edges[i];
	return NULL;
}

void print_nodes(std::vector<node *> &graph){
		cout << "print_nodes() function call output: " << endl;
	int counter = 0;
	for (unsigned int i=0; i<graph.size(); i++){ 
		if (graph[i]->type == PROCESS){
			cout << "NODE types: " << graph[i]->type << " \tPROCESS name: " << graph[i]->name << " \tPath: " << graph[i]->path << endl;
			counter++;
		}else if(graph[i]->type == REG){
			cout << "NODE types: " << graph[i]->type << "     \tFILE    name: " << graph[i]->name << " \tPath: " << graph[i]->path << endl;
			counter++;
		}else if(graph[i]->type == LIBRARY){
			cout << "NODE types: " << graph[i]->type << " \tLIBRARY name: " << graph[i]->name << " \tPath: " << graph[i]->path << endl;
			counter++;
		}else if(graph[i]->type == IPv4_SOCK){
			cout << "NODE types: " << graph[i]->type << " \tfd:inode: " << graph[i]->name << " \taddress(domain:proto): " << graph[i]->path << endl;
			counter++;
		}
	}		
	cout << "\nSummary:: The total node in the graph " << graph.size() << endl;
}
/*
void print_edges(std::vector<node *> &graph){
		cout << "print_edges() function call output: " << endl;
		for (unsigned int i=0; i < graph.size(); i++){
			S
			
		}
}
*/
