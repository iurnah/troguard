/*   
 * Implement the function that used to parse the lttng trace results.
 *   
 * name: 
 * @param argv, the input file name, 
 * @param graph, the initial graph to start with
 * @return detection_point_graph_edege, the address of the edge vector
 * Bug: fcntl change the fd and then close, the fd will be closed, this will cause missing of some read or write match.
 */ 
#include "common.h"
#include "trace_parser.h"
 
static double current_time = 0.0;

//rui__graph_edge *trace_parser(char *argv[], vector <node *> &graph){
graph_edge *trace_parser(char *argv[], multimap <string, node *> &graph){

	string line;

	vector<vector<lttng_open *> > current_opens; 
	vector<vector<lttng_close *> > current_closes;
	vector<vector<lttng_read *> > current_reads;
	vector<vector<lttng_write *> > current_writes;
	vector<vector<lttng_fcntl *> > current_fcntls;
	vector<vector<lttng_socket *> > current_sockets;
	vector<vector<lttng_socket_send *> > current_sock_sends;
	vector<vector<lttng_socket_recv *> > current_sock_recvs;

    for (int i=0; i<4; i++){
        current_opens.push_back(vector<lttng_open *>());
        current_closes.push_back(vector<lttng_close *>());
        current_reads.push_back(vector<lttng_read *>());
        current_writes.push_back(vector<lttng_write *>());
        current_fcntls.push_back(vector<lttng_fcntl *>());
        current_sockets.push_back(vector<lttng_socket *>());
        current_sock_sends.push_back(vector<lttng_socket_send *>());
        current_sock_recvs.push_back(vector<lttng_socket_recv *>());
    }

	ifstream logfile(argv[1]);
	if(!logfile){ cout << "Error: could not open the file!" << endl; exit(1); }

	graph_edge *detection_point_graph_edge = NULL;

    ofstream debugcout;
    debugcout.open("../debug_output.dat");

    int line_number = 0;

	while(getline(logfile, line)){
        line_number++;
        if (line_number%10000 == 0)
            cout << "\r" << line_number << endl;
       
        int index = atoi(get_cpu_id(line).c_str());

        debugcout << line << endl;

		current_time += get_time_stamp(line);

        if(current_time > 60){
        //process all the socket without explicitly closed, give them all the close time 60
            for(unsigned int index=0; index<4;index++){
                for(unsigned int i=0;i<current_sockets[index].size();i++){
                    current_sockets[index][i]->timestamp_end = current_time;
                    add_socket(graph, current_sockets[index][i]);
                }
            }
            break;
        }

        //To terminate the data processing when we get 60 seconds data.
		if(line.find("sys_open:") != string::npos){
		    current_opens[index].push_back(new lttng_open(current_time, get_cpu_id(line),
                                                    get_proc_name(line), get_file_path(line),
                                                    get_file_name(line), "", ""));// "" is fd and retvalue

            debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_open.." << endl;
		}else if(line.find("sys_close:") != string::npos){
            for(unsigned int i=0; i<current_opens[index].size(); i++){
                string tmp_fd = get_fd(line),
                        tmp_cpuid = get_cpu_id(line),
                        tmp_procname = get_proc_name(line);
                if(current_opens[index][i]->fd == tmp_fd
                    && current_opens[index][i]->cpuid == tmp_cpuid
                    && current_opens[index][i]->procname == tmp_procname ){

                    current_closes[index].push_back(new lttng_close(current_time, tmp_cpuid,
                                                                  tmp_procname, tmp_fd, ""));//"" is retvalue
                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: SYS_CLOSE FOR OPEN.." << endl;
                    break;
                }
            }

            for(unsigned int i=0; i<current_sockets[index].size(); i++){
                string tmp_fd = get_fd(line),
                        tmp_cpuid = get_cpu_id(line),
                        tmp_procname = get_proc_name(line);
                if(current_sockets[index][i]->fd == tmp_fd
                    && current_sockets[index][i]->cpuid == tmp_cpuid
                    && current_sockets[index][i]->procname == tmp_procname ){

                    current_closes[index].push_back(new lttng_close(current_time, tmp_cpuid,
                                                                  tmp_procname, tmp_fd, ""));//"" is retvalue
                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: SYS_CLOSE FOR SOCKET.." << endl;
                    break;
                }
            }

		}else if(line.find("sys_read:") != string::npos || line.find("sys_readv:") != string::npos){
            for(unsigned int i=0; i<current_opens[index].size();i++){
                string tmp_fd = get_fd(line),
                        tmp_cpuid = get_cpu_id(line),
                        tmp_procname = get_proc_name(line);
                if(current_opens[index][i]->fd == tmp_fd
                    && current_opens[index][i]->cpuid == tmp_cpuid
                    && current_opens[index][i]->procname == tmp_procname){

                    current_reads[index].push_back(new lttng_read(current_time, tmp_cpuid,
                                                                  tmp_procname, tmp_fd, ""));//"" is retvalue

                    (current_reads[index].back())->filepath = current_opens[index][i]->filepath;
                    (current_reads[index].back())->filename = current_opens[index][i]->filename;

                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_read opened FILE or LIB.." << endl;
                    break;
                }
            }
            //looking for socket to match read if current_open doesn't have a match.
            for(unsigned int i=0; i<current_sockets[index].size();i++){
                string tmp_fd = get_fd(line),
                        tmp_cpuid = get_cpu_id(line),
                        tmp_procname = get_proc_name(line);

                if(current_sockets[index][i]->fd == tmp_fd
                    && current_sockets[index][i]->cpuid == tmp_cpuid
                    && current_sockets[index][i]->procname == tmp_procname){

                    current_reads[index].push_back(new lttng_read(current_time, tmp_cpuid,
                                                                  tmp_procname, tmp_fd, ""));//"" is retvalue

                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_read SOCKET.." << endl;
                    break;
                    //here no need to add the filepath and the filename. because it is reading a socket.
                }
            }
		}else if(line.find("sys_write:") != string::npos || line.find("sys_writev:") != string::npos){

            for(unsigned int i=0; i<current_opens[index].size();i++){
                string tmp_fd = get_fd(line),
                        tmp_cpuid = get_cpu_id(line),
                        tmp_procname = get_proc_name(line);

                if( current_opens[index][i]->fd == tmp_fd
                    && current_opens[index][i]->cpuid == tmp_cpuid
                    && current_opens[index][i]->procname == tmp_procname ){

                    current_writes[index].push_back(new lttng_write(current_time, tmp_cpuid,
                                                                    tmp_procname, tmp_fd, ""));//"" is retvalue

                    (current_writes[index].back())->filepath = current_opens[index][i]->filepath;
                    (current_writes[index].back())->filename = current_opens[index][i]->filename;

                    //cout << "trace_parser:: sys_write.." << endl; getchar();
                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_write FILE .." << endl;

                    break;
                }
            }
            //looking for socket to match read if current_open doesn't have a match.
            for(unsigned int i=0; i<current_sockets[index].size();i++){
                string tmp_fd = get_fd(line),
                        tmp_cpuid = get_cpu_id(line),
                        tmp_procname = get_proc_name(line);
                if(current_sockets[index][i]->fd == tmp_fd
                    && current_sockets[index][i]->cpuid == tmp_cpuid
                    && current_sockets[index][i]->cpuid == tmp_procname){

                    current_writes[index].push_back(new lttng_write(current_time, tmp_cpuid,
                                                                  tmp_procname, tmp_fd, ""));//"" is retvalue
                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_write SOCKET.." << endl;
                    //here no need to add the filepath and the filename. because it is reading a socket.
                    break;
                }
            }

		}else if(line.find("sys_fcntl:") != string::npos){

            if(get_fcntl_cmd(line) == 0 || get_fcntl_cmd(line) == 2){
                current_fcntls[index].push_back(new lttng_fcntl(current_time, get_cpu_id(line),
                                                                get_proc_name(line), get_fd(line),
                                                                get_fcntl_cmd(line), get_fcntl_arg(line), ""));
                //check for whether there is a open.
                for(unsigned int i=0; i<current_opens[index].size();i++){
                    if(current_opens[index][i]->fd == get_fd(line)
                       && current_opens[index][i]->cpuid == get_cpu_id(line)
                       && current_opens[index][i]->procname == get_proc_name(line)){

                        current_opens[index][i]->fd = get_fcntl_arg(line);//fcntl change the fd of in lttng_open.

                        debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_fcntl.." << endl;
                        break;
                    }
                }

            }
        //The match for exit_syscall is correct only that we initialize the "return value" as a blank string "", this return value could mean different things depends on syscall
		}else if(line.find("sys_socket:") != string::npos){

		    current_sockets[index].push_back(new lttng_socket(current_time, 0.0,
                                                            get_cpu_id(line), get_proc_name(line),
                                                            get_socket_family(line), "", ""));

            debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_socket.." << endl;

		}else if(line.find("sys_sendto:") != string::npos
                  || line.find("sys_sendmsg:") != string::npos){//have to match with the opened socket and calculat the total data has been sent for the socket

            debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_sendto or sys_sendmsg.." << endl;

            for(unsigned int i=0;i<current_sockets[index].size();i++){
                string tmp_fd = get_fd(line),
                        tmp_cpuid = get_cpu_id(line),
                        tmp_procname = get_proc_name(line);

                if( current_sockets[index][i]->fd == tmp_fd
                    && current_sockets[index][i]->cpuid == tmp_cpuid
                    && current_sockets[index][i]->procname == tmp_procname ){

                    current_sock_sends[index].push_back(new lttng_socket_send(current_time, tmp_cpuid,
                                                                                tmp_procname, tmp_fd, ""));//"" is retvalue which is a string
                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_sendto or sendmsg matched with corresponding socket.." << endl;
                    break;
                }
            }

        }else if(line.find("sys_recvfrom:") != string::npos
                 || line.find("sys_recvmsg:") != string::npos){

            debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_recvfrom or sys_recvmsg.." << endl;

            for(unsigned int i=0;i<current_sockets[index].size();i++){
                string tmp_fd = get_fd(line),
                        tmp_cpuid = get_cpu_id(line),
                        tmp_procname = get_proc_name(line);

                if( current_sockets[index][i]->fd == tmp_fd
                    && current_sockets[index][i]->cpuid == tmp_cpuid
                    && current_sockets[index][i]->procname == tmp_procname ){

                    current_sock_recvs[index].push_back(new lttng_socket_recv(current_time, tmp_cpuid,
                                                                                tmp_procname, tmp_fd, ""));//"" is retvalue which is a string

                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_recvfrom or recvmsg matched with corresponding socket.." << endl;
                    break;
                }
            }

		}else if(line.find("exit_syscall:") != string::npos){

            //iterate vector for open
            if(current_opens[index].size() > 0){
                for(unsigned int i=0; i<current_opens[index].size();i++){
                    if(current_opens[index][i]->fd == ""
                       && current_opens[index][i]->retvalue == ""
                       && current_opens[index][i]->cpuid == get_cpu_id(line)
                       && current_opens[index][i]->procname == get_proc_name(line)){

                        if(strtol(get_ret(line).c_str(), NULL, 10) < 0){
                            debugcout << "Find open failed!!! (return value is negtive)" << endl;
                            current_opens[index].erase(current_opens[index].begin()+i);
                        }else {
                            current_opens[index][i]->retvalue = get_ret(line);
                            current_opens[index][i]->fd = current_opens[index][i]->retvalue;
                            debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: ( exit_syscall -> sys_open ) matched.." << endl;
                        }
                    break;
                    }
                }
            }

            //iterate vector for close for mactching its exit_syscall,
            if(current_closes[index].size() > 0){
                for(unsigned int j=0; j<current_closes[index].size();j++){
                    if(current_closes[index][j]->retvalue == ""
                       && current_closes[index][j]->cpuid == get_cpu_id(line)
                       && current_closes[index][j]->procname == get_proc_name(line)){

                        if(strtol(get_ret(line).c_str(), NULL, 10) < 0){
                            debugcout << "Find close failed!!! (return value is negtive)" << endl;
                            current_closes[index].resize(remove(current_closes[index].begin(),current_closes[index].end(), current_closes[index][j]) - current_closes[index].begin());
                        }else {
                            current_closes[index][j]->retvalue = get_ret(line);
                            debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: ( exit_syscall -> sys_close ) matched.." << endl;

                            //all the memory allocated for SYS_OPEN should be released
                            for(unsigned int i=0; i<current_opens[index].size();i++){
                                //cout << "inside for" << endl;
                                if(current_opens[index][i]->fd == current_closes[index][j]->fd
                                   && current_opens[index][i]->cpuid == current_closes[index][j]->cpuid
                                   && current_opens[index][i]->procname == current_closes[index][j]->procname){


                                    current_opens[index].resize(remove(current_opens[index].begin(),current_opens[index].end(), current_opens[index][i]) - current_opens[index].begin());
                                    i=-1;
                                    //current_opens[index].erase(current_opens[index].begin()+i);
                                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_close -> erase sys_open.." << endl;
                                }
                            }
                            //all the memory allocated for SYS_READ should be released
                            for(unsigned int i=0;i<current_reads[index].size();i++){
                                if(current_reads[index][i]->fd == current_closes[index][j]->fd
                                   && current_reads[index][i]->cpuid == current_closes[index][j]->cpuid
                                   && current_reads[index][i]->procname == current_closes[index][j]->procname){

                                    current_reads[index].erase(remove(current_reads[index].begin(),current_reads[index].end(), current_reads[index][i]), current_reads[index].end());
                                    i=-1;
                                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_close -> erase sys_read.." << endl;
                                }
                            }
                            //all the memory allocated for SYS_WRITE should be released
                            for(unsigned int i=0; i<current_writes[index].size();i++){
                                if(current_writes[index][i]->fd == current_closes[index][j]->fd
                                   && current_writes[index][i]->cpuid == current_closes[index][j]->cpuid
                                   && current_writes[index][i]->procname == current_closes[index][j]->procname){

                                    current_writes[index].erase(remove(current_writes[index].begin(),current_writes[index].end(), current_writes[index][i]), current_writes[index].end());
                                    i=-1;
                                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_close -> erase sys_write.." << endl;
                                }
                            }
                            //All the memory allocated for SYS_SOCKET should be released
                            for(unsigned int i=0;i<current_sockets[index].size();i++){
                                if(current_sockets[index][i]->fd == current_closes[index][j]->fd
                                   && current_sockets[index][i]->cpuid == current_closes[index][j]->cpuid
                                   && current_sockets[index][i]->procname == current_closes[index][j]->procname){

                                    current_sockets[index][i]->timestamp_end = current_time;

                                    add_socket(graph, current_sockets[index][i]);
                                    current_sockets[index].erase(remove(current_sockets[index].begin(),current_sockets[index].end(), current_sockets[index][i]), current_sockets[index].end());
                                    i=-1;
                                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_close -> erase sys_socket.." << endl
                                               << "add the socket as a node in the graph (call add_socket()) " << endl
                                              << "add the close timestamp for the socket node" << endl;
                                }
                            }
                            //All the memory allocated for SYS_SEND should be released                            for(unsigned int i=0;i<current_sockets[index].size();i++){
                            for(unsigned int i=0;i<current_sock_sends[index].size();i++){
                                if(current_sock_sends[index][i]->fd == current_closes[index][j]->fd
                                   && current_sock_sends[index][i]->cpuid == current_closes[index][j]->cpuid
                                   && current_sock_sends[index][i]->procname == current_closes[index][j]->procname){

                                    current_sock_sends[index].erase(remove(current_sock_sends[index].begin(),current_sock_sends[index].end(), current_sock_sends[index][i]), current_sock_sends[index].end());
                                    i=-1;
                                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_close -> erase sys_sock_sends.." << endl;

                                }
                            }
                            //All the memory allocated for SYS_RECV should be released
                            for(unsigned int i=0;i<current_sock_recvs[index].size();i++){
                                if(current_sock_recvs[index][i]->fd == current_closes[index][j]->fd
                                   && current_sock_recvs[index][i]->cpuid == current_closes[index][j]->cpuid
                                   && current_sock_recvs[index][i]->procname == current_closes[index][j]->procname){

                                    current_sock_recvs[index].erase(remove(current_sock_recvs[index].begin(),current_sock_recvs[index].end(), current_sock_recvs[index][i]), current_sock_recvs[index].end());
                                    i=-1;
                                    debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: sys_close -> erase sys_sock_recvs.." << endl;

                                }
                            }
                        }//else
                    break;
                    }
                }
            }
            //iterate vector for fcntl to match the exit_syscall
            if(current_fcntls[index].size()>0){
                for(unsigned int i=0; i<current_fcntls[index].size();i++){
                    if(current_fcntls[index][i]->retvalue == ""
                       && current_fcntls[index][i]->cpuid == get_cpu_id(line)
                       && current_fcntls[index][i]->procname == get_proc_name(line)){
                        //this assigne value to return value make a difference, so the following read and write wouldn't add new node.
                        current_fcntls[index][i]->retvalue = get_ret(line);

                        debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: ( exit_syscall -> sys_fcntls ) matched.." << endl;
                        break;
                    }
                }
            }

            //iterate vector for read to match the exit_syscall
            if(current_reads[index].size() > 0){
                for(unsigned int i=0; i<current_reads[index].size();i++){
                    if(current_reads[index][i]->retvalue == ""
                       && current_reads[index][i]->cpuid == get_cpu_id(line)
                       && current_reads[index][i]->procname == get_proc_name(line)){
//should distinguish whether it is the read->open or read->socket?

                        if(current_reads[index][i]->filepath == ""){ //socket read doesn't have a filename or filepath.
                            //add to the particular socket read = recv, write = send
                //cout << "this is inside current_reads match conditional sentence." << endl;
                            for(unsigned int j=0; j<current_sockets[index].size();j++){
                                if(current_sockets[index][j]->fd == current_reads[index][i]->fd
                                   && current_sockets[index][j]->cpuid == current_reads[index][i]->cpuid
                                   && current_sockets[index][j]->procname == current_reads[index][i]->procname){

                                        current_reads[index][i]->retvalue = get_ret(line);
                                        current_sockets[index][j]->total_recved += strtol((current_reads[index][i]->retvalue).c_str(),NULL,10);
                                        debugcout << "total_sent value now is " << current_sockets[index][j]->total_sent << endl;
                                        debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: add sendto or sendmsg \"retvalue\" to socket (The socket fd must matched!!!)" << endl;
                                   }
                            }
                        }else{
                            current_reads[index][i]->retvalue = get_ret(line);
                            add_read(graph, current_reads[index][i]);
                            debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: ( exit_syscall -> sys_read ) matched.." << endl << " FILE READ add to the graph.." << endl;

                        }
                        break;
                    }
                }
            }

            //iterate vector for write to match the exit_syscall
            if(current_writes[index].size()>0){
                for(unsigned int i=0; i<current_writes[index].size();i++){
                    if(current_writes[index][i]->retvalue == ""
                       && current_writes[index][i]->cpuid == get_cpu_id(line)
                       && current_writes[index][i]->procname == get_proc_name(line)){

                        if(current_writes[index][i]->filepath == ""){ //socket read doesn't have a filename or filepath.
                            //add to the particular socket read = recv, write = send
                            for(unsigned int j=0; j<current_sockets[index].size();j++){
                                if(current_sockets[index][j]->fd == current_writes[index][i]->fd
                                   && current_sockets[index][j]->cpuid == current_writes[index][i]->cpuid
                                   && current_sockets[index][j]->procname == current_writes[index][i]->procname){

                                        current_writes[index][i]->retvalue = get_ret(line);
                                        current_sockets[index][j]->total_sent += strtol((current_writes[index][i]->retvalue).c_str(),NULL,10);

                                        debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: add sendto or sendmsg \"retvalue\" to socket (The socket fd must matched!!!)" << endl;
                                   }
                            }
                        }else{
                            current_writes[index][i]->retvalue = get_ret(line);
                            add_write(graph, current_writes[index][i]);
                            debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: ( exit_syscall -> sys_read ) matched.." << endl << " FILE READ add to the graph.." << endl;
                        }
                        break;
                    }
                }
            }
            //iterate vector for current_sockets
            if(current_sockets[index].size()>0){
                for(unsigned int i=0; i<current_sockets[index].size();i++){
                    if(current_sockets[index][i]->retvalue == ""
                       && current_sockets[index][i]->fd == ""
                       && current_sockets[index][i]->cpuid == get_cpu_id(line)
                       && current_sockets[index][i]->procname == get_proc_name(line)){

                        if(strtol(get_ret(line).c_str(), NULL, 10) < 0){
                            debugcout << "Find socket open failed!!! (return value is negtive)" << endl;
                            current_sockets[index].erase(current_sockets[index].begin()+i);
                        }else {
                            current_sockets[index][i]->retvalue = get_ret(line);
                            current_sockets[index][i]->fd = current_sockets[index][i]->retvalue;
                            debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: ( exit_syscall -> sys_socket ) matched.." << endl;
                        }
                        //cout << "trace_parser:: exit_syscall -> socket matched.." << endl; getchar();
                        break;
                    }
                }
            }

            //iterate vector for current_sock_sends
            if(current_sock_sends[index].size()>0){
                for(unsigned int i=0; i<current_sock_sends[index].size();i++){
                    if(current_sock_sends[index][i]->retvalue == ""
                       && current_sock_sends[index][i]->cpuid == get_cpu_id(line)
                       && current_sock_sends[index][i]->procname == get_proc_name(line)){

                        current_sock_sends[index][i]->retvalue = get_ret(line);
                        debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: ( exit_syscall -> sys_sendto or sys_sendmsg ) matched.." << endl;
                        //here we don't need to add the sendto or sendmsg to graph, we only need to assign the return value to the correspondence socekt.

                        for(unsigned int j=0; j<current_sockets[index].size();j++){
                            if(current_sockets[index][j]->fd == current_sock_sends[index][i]->fd
                               && current_sockets[index][j]->cpuid == current_sock_sends[index][i]->cpuid
                               && current_sockets[index][j]->procname == current_sock_sends[index][i]->procname){

                                current_sockets[index][j]->total_sent += strtol((current_sock_sends[index][i]->retvalue).c_str(),NULL,10);
                                //debugcout << "total_sent value now is " << current_sockets[index][j]->total_sent << endl;
                                debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: add sendto or sendmsg \"retvalue\" to socket (The socket fd must matched!!!)" << endl;
                               }
                        }
                    break;
                    }
                }
            }
            //iterate vector for current_sock_recvs
            if(current_sock_recvs[index].size()>0){
                for(unsigned int i=0; i<current_sock_recvs[index].size();i++){
                    if(current_sock_recvs[index][i]->retvalue == ""
                       && current_sock_recvs[index][i]->cpuid == get_cpu_id(line)
                       && current_sock_recvs[index][i]->procname == get_proc_name(line)){

                        current_sock_recvs[index][i]->retvalue = get_ret(line);

                        debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: ( exit_syscall -> sys_recvfrom or sys_recvmsg ) matched.." << endl;

                        for(unsigned int j=0; j<current_sockets[index].size();j++){
                            if(current_sockets[index][j]->fd == current_sock_recvs[index][i]->fd
                               && current_sockets[index][j]->cpuid == current_sock_recvs[index][i]->cpuid
                               && current_sockets[index][j]->procname == current_sock_recvs[index][i]->procname){

                               current_sockets[index][j]->total_recved += strtol((current_sock_recvs[index][i]->retvalue).c_str(),NULL,10);
                               debugcout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ trace_parser:: add recvfrom or recvmsg \"retvalue\" to socket (The socket fd must matched!!!)" << endl;
                               }
                        }
                        break;
                    }
                }
            }

        }

	}//while
    cout << "\nTotal node in the graph: " << graph.size() << endl;
	return detection_point_graph_edge;
}


/*
add_read is to add read event to the graph. two node, reader and readee.
*/
//rui__graph_edge *add_read(vector<node *> &graph, lttng_read * current_read){
graph_edge *add_read(multimap <string, node *> &graph, lttng_read * current_read){

    string obj_type = "reg";
    if(current_read->filename.find(".so") != string::npos
       || current_read->filename.find(".mo") != string::npos){
        obj_type = "lib";
    }
    //search graph for the two new nodes;
    node *subj = search_graph(graph, "proc", current_read->procname, "");//"" is the path which should be nothing when create this node
    node *obj = search_graph(graph, obj_type, current_read->filename, current_read->filepath);
    graph_edge *new_edge;

    if (!subj){
        subj = new node(current_read->timestamp, current_read->fd,
                        "proc", current_read->procname);

        graph.insert (pair<string, node *>(subj->type + subj->name + subj->path, subj));
        //graph[(subj->type + subj->name + subj->path)] = subj;
    }

    if(!obj){
        obj =  new node(current_read->timestamp, current_read->fd,
                        obj_type, current_read->filename,
                        current_read->filepath, current_read->retvalue, "");

        graph.insert (pair<string, node *>(obj->type + obj->name + obj->path, obj));


    }else{//if obj is exist, add the retvalue to the data_access amount
        long int sum = strtol(obj->read_bytes.c_str(), NULL, 10) + strtol(current_read->retvalue.c_str(), NULL, 10);
        obj->read_bytes = longToString(sum);
    }
        //every time there is a edge when there is a read action.
        new_edge = new graph_edge(obj,subj,current_read->timestamp, current_read->retvalue);
        subj->parents.push_back(new_edge);
        obj->children.push_back(new_edge);
        //cout << "read edge edge added " << endl;
     return new_edge;//some g
}
/*
add_write is to add read event to the graph. two node, reader and readee.
*/
//rui__graph_edge *add_write(vector<node *> &graph, lttng_write * current_write){
graph_edge *add_write(multimap <string, node *> &graph, lttng_write * current_write){

    string obj_type = "reg";
    if(current_write->filename.find(".so") != string::npos
       || current_write->filename.find(".mo") != string::npos){
        obj_type = "lib";
    }
    //search graph for the two new nodes;
    node *subj = search_graph(graph, "proc", current_write->procname, "");
    node *obj = search_graph(graph, obj_type, current_write->filename, current_write->filepath);
    graph_edge *new_edge;

    if(!subj){
        subj = new node(current_write->timestamp, current_write->fd,
                         "proc", current_write->procname);

        //graph[(subj-> type + subj->name + subj->path)] = subj;
        graph.insert (pair<string, node *>(subj->type + subj->name + subj->path, subj));

    }

    if(!obj){
        obj = new node(current_write->timestamp, current_write->fd,
                        obj_type, current_write->filename,
                        current_write->filepath, "",current_write->retvalue);

        //graph[(obj->type + obj->name + obj->path)] = obj;
        graph.insert (pair<string, node *>(obj->type + obj->name + obj->path, obj));
    }else{
        long int sum = strtol(obj->write_bytes.c_str(), NULL, 10) + strtol(current_write->retvalue.c_str(), NULL, 10);
        obj->write_bytes = longToString(sum);
    }

        new_edge = new graph_edge(subj, obj, current_write->timestamp, current_write->retvalue);
        subj->children.push_back(new_edge);
        obj->parents.push_back(new_edge);

    return new_edge;//some g
}
/*
add_socket object to the graph, different socket type should be add differently
*/
//rui__graph_edge *add_socket(vector<node *> &graph, lttng_socket * current_socket){
graph_edge *add_socket(multimap <string, node *> &graph, lttng_socket * current_socket){

        node *subj = new node(current_socket->timestamp, current_socket->fd,
                         "proc", current_socket->procname);

        //graph[(subj->type + subj->name + subj->path)] = subj;
        graph.insert (pair<string, node *>(subj->type + subj->name + subj->path, subj));

        node *obj = new node(current_socket->timestamp, current_socket->timestamp_end,
                          current_socket->procname, current_socket->type,
                          current_socket->family, current_socket->fd,
                          current_socket->total_sent, current_socket->total_recved);

        string key_for_socket_node = doubleToString(obj->timestamp);
        graph.insert(pair<string, node *>(key_for_socket_node, obj));

        graph_edge *new_edge = new graph_edge(subj,obj,current_socket->timestamp, "");
        subj->children.push_back(new_edge);
        obj->parents.push_back(new_edge);

    return new_edge;
}

/*
search_graph is used for avoiding adding repeated nodes in the graph.
*/
//    node *subj = search_graph(graph, "proc", current_write->procname, "");
//    node *obj = search_graph(graph, obj_type, current_write->filename, current_write->filepath);
//rui__node *search_graph(vector <node *> &graph, string type, string proc_file_name, string path){// reminder: e.g. type=PROCESS
node *search_graph(multimap <string, node *> &graph, string type, string proc_file_name, string path){// reminder: e.g. type=PROCESS

    //cout << "search_graph:: just called.." << endl;

    multimap<string, node *>::iterator it;
    node *return_node;

    //string key = proc_file_name + path;
    string key = type + proc_file_name + path;

    it = graph.find(key);

    if (it !=  graph.end()){
        //cout << "search_graph:: FOUND SUCCESSFULLY.." << endl;
        return_node =  it->second;
    }else{
        //cout << "could not find.." << endl;
        return_node = NULL;
    }

    return return_node;

}


/**
map<string, string> get_attribute(multimap<string, node *> &graph, char *argv[], double point){
cout << "in the function get_attribute: " << endl;
    map <string, string> attr_grp0;
    map <string, string> attr_grp1;
    map <string, string> attr_grp2;
    map <string, string> attr_grp3;
    map <string, string>::iterator it;

    map <string, string> full_attr;

    string period = doubleToString(point*10+5);
    attr_grp0 = get_attr_grp0(argv, point*10, point*10+10);//point should start with 0
    attr_grp1 = get_attr_grp1(graph, point*10, point*10+10);//poit should start with 0
    attr_grp2 = get_attr_grp2(graph, point*10, point*10+10);
    //cout << "This is print the path: " << argv2 <<endl;
    //attr_grp3 = get_attr_grp3(argv, period);//This argv is not same as the one in the get_attr_grp0; this is the path of the socket-ip.dat

    for(it=attr_grp0.begin();it!=attr_grp0.end();++it){
        full_attr[it->first] = it->second;
    }
    for(it=attr_grp1.begin();it!=attr_grp1.end();++it){
        full_attr[it->first] = it->second;
    }
    for(it=attr_grp2.begin();it!=attr_grp2.end();++it){
        full_attr[it->first] = it->second;
    }
    
    for(it=attr_grp3.begin();it!=attr_grp3.end();++it){
        full_attr[it->first] = it->second;
    }
  
    return full_attr;
}
*/


/*MOD*/
map<string, string> get_attribute(multimap<string, node *> &graph, char *argv[]){
cout << "in the function get_attribute: " << endl;
    map <string, string> attr_grp0;
    map <string, string> attr_grp1;
    map <string, string> attr_grp2;
    map <string, string>::iterator it;

    map <string, string> full_attr;

    attr_grp0 = get_attr_grp0(argv);//point should start with 0
    attr_grp1 = get_attr_grp1(graph);//poit should start with 0
    attr_grp2 = get_attr_grp2(graph);

    for(it=attr_grp0.begin();it!=attr_grp0.end();++it){
        full_attr[it->first] = it->second;
    }
    for(it=attr_grp1.begin();it!=attr_grp1.end();++it){
        full_attr[it->first] = it->second;
    }
    for(it=attr_grp2.begin();it!=attr_grp2.end();++it){
        full_attr[it->first] = it->second;
    }
/*    
    for(it=attr_grp3.begin();it!=attr_grp3.end();++it){
        full_attr[it->first] = it->second;
    }
*/    
    return full_attr;
}

map <string, string> get_attr_grp0(char *argv[]){

	double current_time_4count = 0.0;
	cout << "in the function get_attr_grp0: " << endl;
	cout << "argv[1] = " << argv[1] << endl;
	ifstream logfile(argv[1]);

	if(!logfile){
		cout << "Error: could not open the file for count the system call based on the syscall classification !" << endl;
		exit(1);
	}

	string line;
    map <string, string> attribute_grp0;

	unsigned long file_class_count = 0;
	unsigned long mem_class_count = 0;
	unsigned long proc_class_count = 0;
	unsigned long net_class_count = 0;

	unsigned long total_line_count = 0;
	unsigned long exit_syscall_count = 0;

	while (getline(logfile, line)){

        current_time_4count += get_time_stamp(line);
         
//cout << "current_time_4count:: " << current_time_4count << endl;

		total_line_count++;
        //if(have_app(line) || line.find("\"java") != string::npos && low<current_time_4count && high>current_time_4count){//for zdclient
        //if(have_app(line) && low<current_time_4count && high>current_time_4count){
        //if(have_app(line)){
		if (get_proc_name(line) == APPNAME){
            //FILE ACCESS
            if(get_syscall_name(line) == "exit_syscall") { exit_syscall_count++; }
            else if(get_syscall_name(line) == "sys_creat"){ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_creat"){ file_class_count++; }
            else if(get_syscall_name(line) == "sys_open") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_open") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_read") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_read") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_write")		{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_write")		{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_truncate")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_truncate")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_ftruncate")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_ftruncate")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_pipe") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_pipe") 		{ file_class_count++; }
            //File Attributes
            else if(get_syscall_name(line) == "sys_access") 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_access") 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_stat")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_stat")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_fstat")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_fstat")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_lstat")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_lstat")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_umask")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_umask")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_utime")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_utime")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_utimes")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_utimes")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_chmod")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_chmod")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_fchmod")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_fchmod")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_chown")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_chown")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_fchown")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_fchown")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_link")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_link")	 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_symlink") 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_symlink") 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_rename") 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_rename") 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_unlink") 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_unlink") 	{ file_class_count++; }
            //Read/Write
            else if(get_syscall_name(line) == "sys_llseek") 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_llseek") 	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_lseek") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_lseek") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_readlink") 	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_readlink") 	{ file_class_count++; }
            //Directory Operations:
            else if(get_syscall_name(line) == "compat_sys_mkdir") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_mkdir") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_mknod") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_mknod") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_rmdir") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_rmdir") 		{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_getdents")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getdents")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_readdir")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_readdir")	{ file_class_count++; }
            //Miscellaneous
            else if(get_syscall_name(line) == "sys_fdatasync")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_fdatasync")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_fsync")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_fsync")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_mysync")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_mysync")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_chroot")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_chroot")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_chdir")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_chdir")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_fchdir")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_fchdir")	{ file_class_count++; }
            //FD setup
            else if(get_syscall_name(line) == "compat_sys_close")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_close")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_mmap")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_mmap")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_munmap")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_munmap")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_getdtablesize")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getdtablesize")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_dup")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_dup")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_dup2")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_dup2")	{ file_class_count++; }
            //FD Read/Write
            else if(get_syscall_name(line) == "sys_read")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_read")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_readv")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_readv")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_write")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_write")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_writev")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_writev")	{ file_class_count++; }
            //FD control
            else if(get_syscall_name(line) == "sys_flock")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_flock")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_fcntl")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_fcntl")	{ file_class_count++; }
            else if(get_syscall_name(line) == "sys_ioctl")	{ file_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_ioctl")	{ file_class_count++; }
            //NETWORK
            //Setup
            else if(get_syscall_name(line) == "sys_socket")		{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_socket")		{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_socketpair")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_socketpair")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_getsockopt")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getsockopt")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_setsockopt")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setsockopt")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_bind")		{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_bind")		{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_getsockname"){ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getsockname"){ net_class_count++; }
            else if(get_syscall_name(line) == "sys_listen")		{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_listen")		{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_accept")		{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_accept")		{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_connect")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_connect")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_shutdown")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_shutdown")	{ net_class_count++; }
            //Send/Receive
            else if(get_syscall_name(line) == "sys_recv")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_recv")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_recvfrom")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_recvfrom")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_recvmsg")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_recvmsg")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_send")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_send")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_sendto")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sendto")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_sendmsg")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sendmsg")	{ net_class_count++; }
            //Naming
            else if(get_syscall_name(line) == "sys_getdomainname")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getdomainname")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_setdomainname")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setdomainname")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_gethostname")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_gethostname")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_sethostname")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sethostname")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_gethostid")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_gethostid")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_sethostid")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sethostid")	{ net_class_count++; }
            else if(get_syscall_name(line) == "sys_getpeername")	{ net_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getpeername")	{ net_class_count++; }
            //PROCESS CONTROL
            //creation and termination
            else if(get_syscall_name(line) == "sys_exit")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_exit")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_clone")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_clone")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_execve")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_execve")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_fork")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_fork")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_vfork")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_vfork")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_wait")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_wait")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_waitpid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_waitpid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_wait4")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_wait4")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_getpid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getpid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_getppid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getppid")	{ proc_class_count++; }
            //Signal
            //Signal
            else if(get_syscall_name(line) == "sys_kill")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_kill")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_killpg")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_killpg")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_sigblock")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sigblock")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_sigmask")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sigmask")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_siggetmask")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_siggetmask")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_sigsetmask")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sigsetmask")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_signal")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_signal")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_sigvec")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sigvec")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_sigaction")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sigaction")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_sigprocmask")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sigprocmask")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_sigpending")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sigpending")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_sigsuspend")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sigsuspend")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_sigpause")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sigpause")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_pause")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_pause")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_sigreturn")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sigreturn")	{ proc_class_count++; }
            //Synchronization
            else if(get_syscall_name(line) == "sys_poll")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_poll")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_select")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_select")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_semctl")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_semctl")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_semget")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_semget")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_semop")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_semop")	{ proc_class_count++; }
            //User/Group ID
            else if(get_syscall_name(line) == "sys_getuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_getgid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getgid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_getegid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getegid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_geteuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_geteuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_getresuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getresuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_getresgid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getresgid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_getsid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getsid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_getpgid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getpgid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_getpgrp")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getpgrp")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_getgroups")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getgroups")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setegid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setegid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_seteuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_seteuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setfsgid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setfsgid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setfsuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setfsuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setgid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setgid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setregid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setregid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setreuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setreuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setresgid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setresgid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setresuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setresuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setgroups")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setgroups")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setsid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setsid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setuid")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setuid")	{ proc_class_count++; }
            //Resource control
            else if(get_syscall_name(line) == "sys_getrlimit")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getrlimit")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setrlimit")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setrlimit")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_getrusage")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getrusage")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_getpriority")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_getpriority")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_nice")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_nice")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "sys_setpriority")	{ proc_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_setpriority")	{ proc_class_count++; }
            //MEMORY
            //Virtual Memeory
            else if(get_syscall_name(line) == "sys_brk")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_brk")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "sys_sbrk")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_sbrk")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "sys_mlock")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_mlock")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "sys_mlockall")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_mlockall")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "sys_munlock")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_munlock")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "sys_munlockall")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_munlockall")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "sys_mprotect")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_mprotect")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "sys_mremap")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_mremap")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "sys_modify_ldt")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_modify_ldt")	{ mem_class_count++; }
            //Shared Memory
            else if(get_syscall_name(line) == "sys_shmctl")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_shmctl")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "sys_shmat")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_shmat")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "sys_shmdt")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_shmdt")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "sys_shmget")	{ mem_class_count++; }
            else if(get_syscall_name(line) == "compat_sys_shmget")	{ mem_class_count++; }

        }//if
	}
	//}  

	cout << file_class_count << endl;
	cout << net_class_count << endl;
	cout << proc_class_count << endl;
	cout << mem_class_count << endl;

    attribute_grp0["File_access_class_count"]=longToString(file_class_count);
    attribute_grp0["Network_class_count"]=longToString(net_class_count);
    attribute_grp0["Process_control_class_count"]=longToString(proc_class_count);
    attribute_grp0["Memory_class_count"]=longToString(mem_class_count);

    return attribute_grp0; 
}

map <string, string> get_attr_grp1(multimap<string, node *> &graph){
cout << "in the function get_attr_grp1" << endl;
    long Read_bytes = 0; 
    long Write_bytes = 0;
    set <string> Read_dir;
    set <string> Write_dir;
    set <string> Depend;
    set <string>::iterator it_set;
    pair <set<string>::iterator, bool> ret;

    map <string, string> attribute_grp1;
    map <string, string>::iterator it_map;
    multimap <string, node *>::iterator it; 

    for(it = graph.begin(); it != graph.end(); it++){ 
        if(it->second->type == "proc" && it->second->name == APPNAME){
        //if(it->second->type == "proc" &&  it->second->name.find("java") != string::npos){
        //if((it->second->type == "proc" && it->second->name == APPNAME) || it->second->name.find("java") != string::npos){
        //if(it->second->type == "proc" && it->second->name.find("hrome") != string::npos){
        //if(it->second->type == "proc" && it->second->name.find("hrom") != string::npos){	//for chromium+Chrome_IOThread
        //if(it->second->type == "proc" && it->second->name.find(APPNAME) != string::npos){
        //if(it->second->type == "proc" && it->second->name.find(APPNAME) != string::npos || it->second->name.find("lowriter") != string::npos || it->second->name.find("soffice") != string::npos || it->second->name.find("oosplash") != string::npos){
            //cout << "am I run?" << endl;
            for(unsigned i=0; i<it->second->parents.size(); i++){
                //cout << it->second->parents[i]->timestamp << endl;
                //if (low < it->second->parents[i]->timestamp && it->second->parents[i]->timestamp < high){
                    Read_bytes += strtol((it->second->parents[i]->bytes).c_str(),NULL,10);
                    Read_dir.insert(get_root_dir(it->second->parents[i]->src_node->path));
                //} //if

            }
            for(unsigned j=0;j<it->second->children.size();j++){
                //if (low < it->second->children[j]->timestamp && it->second->children[j]->timestamp < high){
                    Write_bytes += strtol((it->second->children[j]->bytes).c_str(),NULL,10);
                    Write_dir.insert(get_root_dir(it->second->children[j]->dest_node->path));//need to be removed
                    //cout << Write_bytes << endl;
                //} //if
            }
        } 
        // find the dependence  
        if(it->second->type == "reg" || it->second->type == "lib"){
            for(unsigned int j=0; j<it->second->parents.size(); j++){
                for(unsigned int i=0; i<it->second->children.size();i++){
									
                    //if (low < it->second->parents[j]->timestamp && it->second->parents[j]->timestamp < high
                    //    && low < it->second->children[i]->timestamp && it->second->children[i]->timestamp < high){
 
                         
                        if ((it->second->parents[j]->src_node->name == APPNAME
                        //if ((it->second->parents[j]->src_node->name == "java"
                        //if ((it->second->parents[j]->src_node->name == APPNAME || it->second->parents[j]->src_node->name == "java"
                        //if ((it->second->parents[j]->src_node->name.find("hrome") != string::npos //for chrome+Chrome_IOThread
                        //if ((it->second->parents[j]->src_node->name.find("hrom") != string::npos // for chromium+Chrome_IOThread
                        //if ((it->second->parents[j]->src_node->name.find(APPNAME) != string::npos
                        //if ((it->second->parents[j]->src_node->name == APPNAME || it->second->parents[j]->src_node->name.find("lowriter") != string::npos || it->second->parents[j]->src_node->name.find("soffice") != string::npos || it->second->parents[j]->src_node->name.find("oosplash") != string::npos
                             || it->second->children[i]->dest_node->name == APPNAME )
                             //|| it->second->children[i]->dest_node->name == "java")
                             //|| it->second->children[i]->dest_node->name == APPNAME || it->second->children[i]->dest_node->name == "java")
                             //|| it->second->children[i]->dest_node->name.find("hrome") != string::npos) //for chrome+Chrome_IOThread
                             //|| it->second->children[i]->dest_node->name.find("hrom") != string::npos) //for chromium+Chrome_IOThread
                             //|| it->second->children[i]->dest_node->name.find(APPNAME) != string::npos)
                             //|| it->second->children[i]->dest_node->name == APPNAME || it->second->children[i]->dest_node->name.find("lowriter") != string::npos || it->second->children[i]->dest_node->name.find("soffice") != string::npos || it->second->children[i]->dest_node->name.find("oosplash") != string::npos)
                             && it->second->parents[j]->src_node->name != it->second->children[i]->dest_node->name
                             && it->second->parents[j]->timestamp < it->second->children[i]->timestamp ){

							if(it->second->parents[j]->src_node->name == APPNAME)
							//if(it->second->parents[j]->src_node->name == "java")
							//if(it->second->parents[j]->src_node->name == APPNAME || it->second->parents[j]->src_node->name == "java")
							//if(it->second->parents[j]->src_node->name == APPNAME || it->second->parents[j]->src_node->name == "java")
                            //if(it->second->parents[j]->src_node->name.find("hrome") != string::npos) //for chrome+Chrome_IOThread
                            //if(it->second->parents[j]->src_node->name.find("hrom") != string::npos) //for chromium+Chrome_IOThread 
                            //if(it->second->parents[j]->src_node->name.find(APPNAME) != string::npos)
                            //if(it->second->parents[j]->src_node->name == APPNAME || it->second->parents[j]->src_node->name.find("lowriter") != string::npos || it->second->parents[j]->src_node->name.find("soffice") != string::npos || it->second->parents[j]->src_node->name.find("oosplash") != string::npos)
                                Depend.insert(it->second->children[i]->dest_node->name);
                            else
                                Depend.insert(it->second->parents[j]->src_node->name);
                        }
                     //} //commented if
                }
            }
        }
        //combine the attributes
        for(it_set = Read_dir.begin();it_set!=Read_dir.end();it_set++){
            attribute_grp1.insert(pair<string,string>("R("+(*it_set)+"/..)","true"));
        }
        for(it_set = Write_dir.begin();it_set!=Write_dir.end();it_set++){
            attribute_grp1.insert(pair<string,string>("W("+(*it_set)+"/..)", "true"));
        }
        for(it_set = Depend.begin();it_set!=Depend.end();it_set++){
            attribute_grp1.insert(pair<string,string>("Dep("+(*it_set)+")","true"));
        }

        attribute_grp1["Total_read"] = longToString(Read_bytes);
        attribute_grp1["Total_write"] = longToString(Write_bytes);

    }
    /*
    //just for debugging
    for(it_map = attribute_grp1.begin(); it_map != attribute_grp1.end(); it_map++){
        cout <<"output: " << it_map->first << " " << it_map->second << endl;
    }
    */
    return attribute_grp1;
}

map <string, string> get_attr_grp2(multimap<string, node *> &graph){
cout << "in the function get_attr_grp2" << endl;
    long Send_bytes = 0;
    long Recv_bytes = 0;

    multiset<string> socket_type; 
    multiset<string>::iterator it_set;


    map<string, string> attribute_grp2;
    map<string, string>::iterator it_map;

    multimap <string, node *>::iterator it;

    for(it = graph.begin(); it != graph.end(); it++){
        
        if(it->second->type == "sock" && it->second->name == APPNAME){
	    //if(it->second->type == "sock" && it->second->name == "java"){
        //if((it->second->type == "sock" && it->second->name == APPNAME) || it->second->name == "java"){
        //if(it->second->type == "sock" && it->second->name.find("hrome") != string::npos){ 	//for chrome+Chrome_IOThread
        //if(it->second->type == "sock" && it->second->name.find("hrom") != string::npos){ 		//for chromium+Chrome_IOThread
        //if(it->second->type == "sock" && it->second->name.find(APPNAME) != string::npos){
		//if(it->second->type == "sock" && it->second->name == APPNAME || it->second->name.find("lowriter") != string::npos || it->second->name.find("soffice") != string::npos || it->second->name.find("oosplash") != string::npos){
//cout << "inside if" << endl;
//cout << "::it->first::" << it->first << endl;
            //if(low < it->second->parents[0]->timestamp && high > it->second->parents[0]->timestamp){
//cout << "it's weired: " << endl;
                Send_bytes += it->second->send_data;
                Recv_bytes += it->second->recv_data;
                socket_type.insert(it->second->family);
            //}  //if
            
        }
    }

    for(it_set=socket_type.begin(); it_set!=socket_type.end();++it_set){        
//cout << "inside for " << endl;
        attribute_grp2[get_sock_type(*it_set)] = intToString(socket_type.count(*it_set));
    }
//cout << "what's gone wrong?" << endl;
    attribute_grp2["Total_sent"] = longToString(Send_bytes);
    attribute_grp2["Total_recieve"] = longToString(Recv_bytes);
/*
    for(it_map = attribute_grp2.begin();it_map != attribute_grp2.end();it_map++){
        cout << "output_2: " << it_map->first << " " << it_map->second << endl;
    }
*/
    return attribute_grp2;
}

/**********************************************************************
map <string, string> get_attr_grp3(char *argv[], string period){
cout << "in the function get_attr_grp3: " << endl;

    string line;
    string flag = "0";
    ifstream socket_sample(argv[2]);
	if(!socket_sample){
		cout << "Error: could not open the socket sample file!" << endl;
		exit(1);
	}
cout << "sample file opened successfully" << endl;

	map<string, string> attribute_grp3;

    multimap<string, string> ip;
    multimap<string, string> port;
    multimap<string, string>::iterator it;

    set <string> ip_set;
    set <string>::iterator it_set;

    multiset <string> port_set;
    multiset <string>::iterator it_mset;

	while(getline(socket_sample, line)){
		//cout << line << endl;
        if(line.find("NEW ITERATION") != string::npos){
            flag=get_sample_period(line);
			//cout << "NEW ITERATION Flag = " << get_sample_period(line) << endl;
        }else if(line.find("NEW ITERATION") == string::npos && flag == "5"){
            //cout << "flag == 5" << endl;
            ip.insert(pair<string,string>(flag,get_ip(line)));
            port.insert(pair<string,string>(flag,get_port(line)));
        }else if(line.find("NEW ITERATION") == string::npos && flag == "15"){
            ip.insert(pair<string,string>(flag,get_ip(line)));
            port.insert(pair<string,string>(flag,get_port(line)));
        }else if(line.find("NEW ITERATION") == string::npos && flag == "25"){
            ip.insert(pair<string,string>(flag,get_ip(line)));
            port.insert(pair<string,string>(flag,get_port(line)));
        }else if(line.find("NEW ITERATION") == string::npos && flag == "35"){
            ip.insert(pair<string,string>(flag,get_ip(line)));
            port.insert(pair<string,string>(flag,get_port(line)));
        }else if(line.find("NEW ITERATION") == string::npos && flag == "45"){
            ip.insert(pair<string,string>(flag,get_ip(line)));
            port.insert(pair<string,string>(flag,get_port(line)));
        }else if(line.find("NEW ITERATION") == string::npos && flag == "55"){
            ip.insert(pair<string,string>(flag,get_ip(line)));
            port.insert(pair<string,string>(flag,get_port(line)));
        }

	}

	for(it=ip.begin(); it!=ip.end();it++){
		if((*it).first == period){
			ip_set.insert((*it).second);
		}
	}
	attribute_grp3["Unique_IPs"] = intToString(ip_set.size());

	for(it=port.begin();it!=port.end();it++){
		if((*it).first == period){
			port_set.insert((*it).second);
		}
	}

	for(it_mset=port_set.begin();it_mset!=port_set.end();it_mset++){
		//attribute_grp3.insert(pair<string,string>("Port("+(*it_mset)+")", intToString(port_set.count(*it_mset))));
		attribute_grp3.insert(pair<string,string>("Port("+(*it_mset)+")", "true"));
	}

	return attribute_grp3;
}
*/

string get_sample_period(string line){

	int start = line.find("NEW ITERATION ") + 14;
    int i=start;

	while(isdigit(line.c_str()[i])) i++;
	int end = i;

	int length = end - start;
	string period = line.substr(start, length);

	return period;
}

string get_ip(string line){
    int end = line.find_first_of(":", 45);
    int length = end - 45;
    string ip = line.substr(45, length);

    return ip;
}

string get_port(string line){
    int start = line.find_first_of(":", 45) + 1;
    int i=start;

    while(isdigit(line.c_str()[i])) i++;
    int end = i;

    int length = end - start;
    string port = line.substr(start, length);

    return port;

}



double get_time_stamp(string line){
	int start = line.find("(+") + 2;
	int end = line.find_first_of(")", start);
	int length = end - start;
	string time = line.substr(start, length);
	char *pEnd;

	double a = strtod(time.c_str(), &pEnd);

	return a;
}

string get_cpu_id(string line){
	int start = line.find("cpu_id = ") + 9;
	int end = line.find_first_of(" }", start);
	int length = end - start;
	string id = line.substr(start, length);

	return id;
}

string get_proc_name(string line){
	int start = line.find("procname = \"") + 12;
	int end = line.find_first_of("\"", start);
	int length = end - start;
	string proc_name = line.substr(start, length);

	return proc_name;
}

string get_file_path(string line){
	int start = line.find("filename = \"") + 12;
	int end = line.find_first_of("\"", start);
	int length = end - start;
	string file_path = line.substr(start, length);

	return file_path;
}

string get_file_name(string line){
	string filepath =  get_file_path(line);
	string filename = filepath.substr(filepath.rfind("/") + 1);

	return filename;
}

string get_fd(string line){
	int start = line.find("fd = ") + 5;
    int i=start;
    //char str[]=line.c_str();
	while(isdigit(line.c_str()[i])) i++;
	int end = i;
	//int end = line.find_first_of(",", start);
	int length = end - start;
	string fd = line.substr(start, length);

	return fd;
}

string get_ret(string line){
	int start = line.find("ret = ") + 6;
	int end = line.find_first_of(" }", start);
	int length = end - start;
	string ret = line.substr(start, length);

    string positive = (ret.find("-") != string::npos)? "0":ret;

    //int a = atoi(ret.c_str());
	return positive;
}

int get_fcntl_cmd(string line){
	int start = line.find("cmd = ") + 6;
	int end = line.find_first_of(" }", start);
	int length = end - start;
	string cmd = line.substr(start, length);

	return atoi(cmd.c_str());
}

string get_fcntl_arg(string line){
	int start = line.find("arg = ") + 6;
	int end = line.find_first_of(" }", start);
	int length = end - start;
	string arg = line.substr(start, length);

	return arg;
}

string get_socket_family(string line){
    int start = line.find("family = ") + 9;
    int end = line.find_first_of(",", start);
    int length = end -start;
    string family = line.substr(start, length);
    //cout << "family>" << family << "<" << endl;
    return family;

}

string get_root_dir(string path){
	string root_dir;
	if(path.find("/") != string::npos){
        int index_of_end = path.find_first_of("/", 1);
        root_dir = path.substr(1, index_of_end-1);
	}else
        root_dir = path;


	return root_dir;
}

string doubleToString(double val){
    ostringstream out;
    out << val;
    return out.str();
}

string longToString(long int val){
    ostringstream out;
    out << val;
    return out.str();
}

string intToString(unsigned int val){
    ostringstream out;
    out << val;
    return out.str();
}


bool have_app(string line){
	if(line.find(APPNAME) != string::npos)
		return true;
	return false;
}

string get_syscall_name(string line){
	//int index_of_start = 36; //count the index in the log file
	int index_of_start = 43; //count the index in the log file
	int index_of_end = line.find_first_of(":", 43);
	int syscall_name_len = index_of_end - index_of_start;
	return line.substr(index_of_start, syscall_name_len);
}

string get_sock_type(string family){
    if(family == "1"){
        return "UNIX";
    }else if(family == "2"){
        return "INET";
    }else if(family == "3"){
        return "AX25";
    }else if(family == "4"){
        return "IPX";
    }else if(family == "5"){
        return "APPLETALK";
    }else if(family == "6"){
        return "NETROM";
    }else if(family == "7"){
        return "BRIDGE";
    }else if(family == "8"){
        return "ATMPVC";
    }else if(family == "9"){
        return "X25";
    }else if(family == "10"){
        return "INET6";
    }else if(family == "11"){
        return "ROSE";
    }else if(family == "12"){
        return "DECnet";
    }else if(family == "13"){
        return "NETBEUI";
    }else if(family == "14"){
        return "SECURITY";
    }else if(family == "15"){
        return "KEY";
    }else if(family == "16"){
        return "NETLINK";
    }else if(family == "17"){
        return "PACKET";
    }else if(family == "18"){
        return "ASH";
    }else if(family == "19"){
        return "ECONET";
    }else if(family == "20"){
        return "ATMSVC";
    }else if(family == "21"){
        return "RDS";
    }else if(family == "22"){
        return "SNA";
    }else if(family == "23"){
        return "IRDA";
    }else if(family == "24"){
        return "PPPOX";
    }else if(family == "25"){
        return "WANPIPE";
    }else if(family == "26"){
        return "LLC";
    }else if(family == "29"){
        return "CAN";
    }else if(family == "30"){
        return "TIPC";
    }else if(family == "31"){
        return "BLUETOOTH";
    }else if(family == "32"){
        return "IUCV";
    }else if(family == "33"){
        return "RXRPC";
    }else if(family == "34"){
        return "ISDN";
    }else if(family == "35"){
        return "PHONET";
    }else if(family == "36"){
        return "IEEE802154";
    }else if(family == "37"){
        return "CAIF";
    }else if(family == "38"){
        return "ALG";
    }else if(family == "39"){
        return "NFC";
    }else{
        return "UNKNOWN-SOCKET-TYPE";
    }
}

string get_attr_name(string line){
	int index_of_string_start = line.find("@attribute ")+11;
	int index_of_string_end = line.find_first_of(" ", index_of_string_start);
	int attr_name_length = index_of_string_end - index_of_string_start;
	string attr_name = line.substr(index_of_string_start, attr_name_length);	

	return attr_name;
}

string search_attribute(vector <attribute_node *> &attributes, string attr_line){
	string name = get_attr_name(attr_line);
	bool find = false;
	string line;
	unsigned int i=0;
	for(unsigned int i=0; i<attributes.size(); i++){
		if(attributes[i]->attribute_name == name){
			line = attributes[i]->attribute_line; 
			find = true;
			break;
		}
	}
	
	if(find){
		if(line.find("INET6 ") != string::npos){
			int start = line.find("INET6 ") + 6;
			return line.substr(start);
		}else if(line.find("true") != string::npos){
			return "true";	
		}else if(line.find("false") != string::npos){
			return "false";
		}else{
			while(!isdigit(line.c_str()[i])) i++;
			int j = i;
			while(isdigit(line.c_str()[i])) i++;
			i++;
			while(isdigit(line.c_str()[i])) i++;
			return (line.find("e-") != string::npos) ? "0" : line.substr(j,i-j);
		/*  if(line.find("e-") != string::npos)
				return "0";
			else if(line.substr(j,i-j).find("6 ") != string::npos)
				return line.substr(j+2,i-j-2);
			else
				return line.substr(j,i-j);
		*/
		}
	}else{
		if(attr_line.find("(") != string::npos
			|| attr_line.find("KB_") != string::npos
			|| attr_line.find("Mouse_") != string::npos){
			return "false";
		}else
			return "0";
	}
}
