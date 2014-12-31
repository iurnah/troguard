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

template <class T>
string ToString(T i){
	ostringstream buffer;
	buffer << i;
	return buffer.str();
}

bool is_number(const string& s){
	for (unsigned int i = 0; i < s.length(); i++)
		if (!isdigit(s[i]))
			return false;
	return true;
}

graph_edge *syslog_parser(char *argv[], std::vector<node *> &graph){

	ifstream if1(argv[1]);
	
	if (!if1){
		cout << "Error:: Could not open the file!" << endl;
		exit(1);
	}
	
	unsigned long time = 0, tmp_pipe_id = 0;
	string line;
	
	string tmp, command, syscall, process, pid, proc_seccon, object_type, fd, file_path, object_name, subject_name, inode_id,
			s_addr, d_addr, // used for AF_INET sockets
			peer_inode_id, // used for AF_UNIX
			nrbufs, curbuf, // used for FIFO objects (named pipes)
			debug_1; // used to ignore corrupted log lines.
	
	gettimeofday(&start, NULL);
	graph_edge *detection_point_graph_edge = NULL;
	int unrecognized_objtype = 0;
	
	while(!if1.eof()){
		time++;
		
		getline(if1, line);
		if (line.find("::Saman::") == string::npos)
			continue;
		
		// parsing the line
		
		line = line.substr(line.find("::Saman::")); // line_substring starting from ::Saman::
		
		istringstream strm(line);
		
		// To get rid of corrupted log lines.
		//strm >> tmp >> command >> process >> pid >> proc_seccon >> object_type;
		strm >> tmp >> debug_1;
		if (debug_1 != DEBUG_COMMAND) continue;
		//strm >> command >> debug_1;
		strm >> command >> syscall >> debug_1;
		if (debug_1 != DEBUG_PROCESS) continue;
		strm >> process >> debug_1;
		if (debug_1 != DEBUG_PID) continue;
		strm >> pid >> debug_1;
		if (debug_1 != DEBUG_SECCON) continue;
		strm >> proc_seccon  >> debug_1;
		if (debug_1 != DEBUG_OBJTYPE) continue;
		strm >> object_type;
		
		//subject_name = process + "<" + pid + ">"; 
		subject_name = process;

		// parsing security_context, e.g., unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c255
		int tmp_int1 = proc_seccon.find(":"),
			tmp_int2 = proc_seccon.find(":", tmp_int1 + 1),
			tmp_int3 = proc_seccon.find(":", tmp_int2 + 1);
		proc_seccon = proc_seccon.substr(tmp_int2 + 1, tmp_int3 - tmp_int2 - 1);
		
		if (proc_seccon.find("_t") == string::npos)
			continue;
		
		
		if (object_type == DIR || object_type == CHR){
			
			continue;
		
		}else if (object_type == REG){
			
			strm >> debug_1;
			if (debug_1 != DEBUG_FD) continue;
			strm >> fd >> debug_1;
			if (debug_1 != DEBUG_INODE) continue;
			strm >> inode_id >> debug_1;
			if (debug_1 != DEBUG_FILEPATH) continue;
			strm >> file_path;
			
			size_t index = 0;
			// if MAX_FILE_SLASH_DEPTH+1 slashes are found, cut! otherwise leave it as is.
			for (unsigned int i=0; i<MAX_FILE_SLASH_DEPTH+1; i++){
				
				if (file_path.find("/", index) == string::npos){
					 break;
				
				}else{
					index = file_path.find("/", index) + 1;	
					if (i == MAX_FILE_SLASH_DEPTH)
						file_path = file_path.substr(0,index);
				}
			}
			
			object_name = file_path;
			if (object_name.find("/3188/") != string::npos){
				cout << "STIME::" << time << endl;
				int xx; cin >> xx;
			}
			
		}else if (object_type == FIFO){
			
			strm >> debug_1;
			if (debug_1 != DEBUG_FD) continue;
			strm >> fd >> debug_1;
			if (debug_1 != DEBUG_INODE) continue;
			strm >> inode_id >> debug_1;
			if (debug_1 != DEBUG_NRBUFS) continue;
			strm >> nrbufs >> debug_1;
			if (debug_1 != DEBUG_CURBUF) continue;
			strm >> curbuf; 
			object_name = object_type + "<" + inode_id + ">:" + ToString(tmp_pipe_id);
			
			// this "if" should not be commented!
			if (command == WRITE  &&  nrbufs == "0"){// we have to remove the pipe with the same id if command="write" and nrbufs=0
				for (unsigned int i=0; i<graph.size(); i++){
					if (graph[i]->name == object_name){
						tmp_pipe_id++;
						object_name = object_type + "<" + inode_id + ">:" + ToString(tmp_pipe_id);
						break;
					}
				}
			}
			
		}else if (object_type == SOCK_AF_INET){
			
			strm >> fd >> inode_id >> s_addr >> d_addr;
			
			// we consider all AF_INET connections as a single external entity!
			//object = object_type + "<" + s_addr + "," + d_addr + ">";
			
			object_name = object_type;
			
		}else if (object_type == SOCK_AF_INET6){
			
			// for INET6 currently there is nothing logged.
			//strm >> fd >> inode_id >> s_addr >> d_addr; 
			
			// we consider all AF_INET connections as a single external entity!
			//object = object_type + "<" + s_addr + "," + d_addr + ">";
			
			object_name = SOCK_AF_INET; // sockets are all (INET and INET6) treated similarly.
			object_type = SOCK_AF_INET;
			
		}else if (object_type == SOCK_AF_UNIX){
			
			strm >> debug_1;
			if (debug_1 != DEBUG_FD) continue;
			strm >> fd >> debug_1;
			if (debug_1 != DEBUG_INODE) continue;
			strm >> inode_id >> debug_1;
			if (debug_1 != DEBUG_INODE) continue;
			strm >> peer_inode_id;
			
			if (!is_number(fd) || !is_number(inode_id) || !is_number(peer_inode_id)) continue;
			
			if (inode_id.compare(peer_inode_id) < 1)
				object_name = object_type + "<" + inode_id + "," + peer_inode_id + ">";
			else
				object_name = object_type + "<" + peer_inode_id + "," + inode_id + ">";
			
			if (object_name == "SOCK_AF_UNIX<10893,1089r_t:s0-s0:c0.c255>"){
				cout << "LINE:: " << time << endl;
				cout << atoi(fd.c_str()) << " " << atoi(inode_id.c_str()) << " " << atoi(peer_inode_id.c_str()) << endl;
				exit(1);
			}
			
			//object = object_type; // this should be commented!
		
		}else if (object_type == IPCMSG){
			
			strm >> inode_id;
			object_name = object_type + "<" + inode_id + ">";
		
		}else if (object_type == PROCESS){ // e.g. sys_execve()
			
			string temp_str;
			strm >> temp_str; // the filename (including '/'s) from which the process is spawned.
			if (temp_str.rfind('/') == string::npos)
				object_name = temp_str;
			else
				object_name = temp_str.substr(temp_str.rfind('/')+1);
			
		}else{
			unrecognized_objtype++;
			//cout << "Saman:: I do not know this type: " << object_type << " line:" << time << endl;
			//int x;cin >> x;
			continue;
		}
		
		if (time%100000 == 0)
			cout << "line:" << time << endl;
		
		node *subj = search_graph(graph, subject_name, PROCESS);
		node *obj = search_graph(graph, object_name, object_type);
		
		if (!subj){
			subj = new node(PROCESS, subject_name);
			graph.push_back(subj);			
		}
		// these are out of "if" because if they do not have 
		// these values, e.g., pid, if the subject was created
		// by an execve systemcall.
		subj->pid = pid;
		subj->seccon = proc_seccon;
		
		
		if (!obj){
			obj = new node(object_type, object_name);
			if (object_type == REG){
				obj->fd = fd;
				obj->inode_id = inode_id;
			}else if (object_type == FIFO){
				obj->fd = fd;
				obj->inode_id = inode_id;
				obj->nrbufs = nrbufs;
				obj->curbuf = curbuf;
			}else if (object_type == SOCK_AF_UNIX){
				obj->fd = fd;
				obj->inode_id = inode_id;
				obj->peer_inode_id = peer_inode_id;
			}else if (object_type == SOCK_AF_INET){
				obj->fd = fd;
				obj->inode_id = inode_id;
				obj->s_addr = s_addr;
				obj->d_addr = d_addr;
			}else if (object_type == IPCMSG){
				obj->inode_id = inode_id;
			}else if (object_type == PROCESS){
				obj->pid = pid; 
			}
			graph.push_back(obj);
		}
		
		graph_edge *e;
		
		if (command == WRITE){
			e = search_graph_edges(subj->children, obj->name);
			if(!e){
				e = new graph_edge(subj, obj, time);
				subj->children.push_back(e);
				obj->parents.push_back(e);
			}else
				e->times.push_back(time);

		}else if (command == READ){
			e = search_graph_edges(obj->children, subj->name);
			if(!e){
				e = new graph_edge(obj, subj, time);
				obj->children.push_back(e);
				subj->parents.push_back(e);
			}else
				e->times.push_back(time);
		}
		
		if (e->src_node == e->dest_node){
			cout << "Error:: self loop found in line " << time << "." << endl;
			exit(1);
		}
		
		if ((int)time == atoi(argv[2])){
			detection_point_graph_edge = e;
			cout << "detection_point:\n\t" << line << endl;
			break;
		}
		
		//cout << "e->times.size(): " << e->times.size() << "\ttime: " << time << "\tgraph_edge: " << e->src_node->name << "->" << e->dest_node->name << endl;
		
		
		// begin.saman this is for experiments
		/*
		ofstream of6("node_convergence.dat", ios::app);
		ofstream of7("weight_convergence.dat", ios::app);
		
		if (time%50000 == 0){
			cout << "starting.." << endl;
			refine_graph(graph);
			float dif2 = node_behavoir_estimator(graph);
			of6 << time << "\t" << graph.size() << endl;
			of7 << time << "\t" << fabs(dif - dif2)/graph.size() << endl;
			cout << "done: " << graph.size() << " " << fabs(dif - dif2)/graph.size() << endl;
			dif = dif2;
		}
		
		of6.close();
		of7.close();
		
		//if (time > 500000) break;
		*/
		// end.saman
		
	}
	if1.close();
	
	gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;	
	
	cout << "\nSaman:: parsing the file (while loop) is Done: \n\t" 
		<< mtime << " msecs \n\t"
		<<  unrecognized_objtype << " of " << time << " lines had unknown object types!" << endl;
	
	return detection_point_graph_edge;
}
