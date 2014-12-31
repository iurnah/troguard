#include "common.h"
#include "trace_parser.h"
 
//summary of the statistics of the system call
//ToDos: have to check some of the system call didn't have the comm ="" field
void syscall_statis(char *argv[]){

	ifstream logfile(argv[1]);

	if(!logfile){
		cout << "Error: could not open the file!" << endl;
		exit(1);
	}
	  
	string line;
	
	unsigned long file_class_count = 0;
	unsigned long mem_class_count = 0;
	unsigned long proc_class_count = 0;
	unsigned long net_class_count = 0;
	unsigned long total_line_count = 0;
	unsigned long exit_syscall_count = 0;
	unsigned long total_app_count = 0;
	 
	ofstream syscall_statis;
	syscall_statis.open("./results/"APPNAME"/syscall_statis.dat");
	
	while (!logfile.eof()){
		getline(logfile, line);
		total_line_count++;
		
		if(have_app(line)){
			total_app_count++;
			//FILE ACCESS
			//Setup 
			if(get_syscall_name(line) == "exit_syscall") { exit_syscall_count++; }
			else if(get_syscall_name(line) == "sys_creat"){ file_class_count++; }
			else if(get_syscall_name(line) == "sys_open") 		{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_read") 		{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_write")		{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_truncate")	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_ftruncate")	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_pipe") 		{ file_class_count++; }
			//File Attributes
			else if(get_syscall_name(line) == "sys_access") 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_stat")	 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_fstat")	 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_lstat")	 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_umask")	 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_utime")	 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_utimes")	 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_chmod")	 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_fchmod")	 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_chown")	 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_fchown")	 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_link")	 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_symlink") 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_rename") 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_unlink") 	{ file_class_count++; }
			//Read/Write
			else if(get_syscall_name(line) == "sys_llseek") 	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_lseek") 		{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_readlink") 	{ file_class_count++; }
			//Directory Operations:
			else if(get_syscall_name(line) == "sys_mkdir") 		{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_mknod") 		{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_rmdir") 		{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_getdents")	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_readdir")	{ file_class_count++; }
			//Miscellaneous
			else if(get_syscall_name(line) == "sys_fdatasync")	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_fsync")	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_mysync")	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_chroot")	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_chdir")	{ file_class_count++; }
			else if(get_syscall_name(line) == "sys_fchdir")	{ file_class_count++; }
			//FD setup
			else if(get_syscall_name(line) == "sys_close")	{ file_class_count++; }			
			else if(get_syscall_name(line) == "sys_mmap")	{ file_class_count++; }			
			else if(get_syscall_name(line) == "sys_munmap")	{ file_class_count++; }			
			else if(get_syscall_name(line) == "sys_getdtablesize")	{ file_class_count++; }			
			else if(get_syscall_name(line) == "sys_dup")	{ file_class_count++; }			
			else if(get_syscall_name(line) == "sys_dup2")	{ file_class_count++; }			
			//FD Read/Write
			else if(get_syscall_name(line) == "sys_read")	{ file_class_count++; }			
			else if(get_syscall_name(line) == "sys_readv")	{ file_class_count++; }			
			else if(get_syscall_name(line) == "sys_write")	{ file_class_count++; }			
			else if(get_syscall_name(line) == "sys_writev")	{ file_class_count++; }		
			//FD control
			else if(get_syscall_name(line) == "sys_flock")	{ file_class_count++; }			
			else if(get_syscall_name(line) == "sys_fcntl")	{ file_class_count++; }			
			else if(get_syscall_name(line) == "sys_ioctl")	{ file_class_count++; }
			//NETWORK
			//Setup
			else if(get_syscall_name(line) == "sys_socket")		{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_socketpair")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_getsockopt")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_setsockopt")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_bind")		{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_getsockname"){ net_class_count++; }
			else if(get_syscall_name(line) == "sys_listen")		{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_accept")		{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_connect")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_shutdown")	{ net_class_count++; }
			//Send/Receive
			else if(get_syscall_name(line) == "sys_recv")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_recvfrom")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_recvmsg")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_send")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_sendto")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_sendmsg")	{ net_class_count++; }
			//Naming
			else if(get_syscall_name(line) == "sys_getdomainname")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_setdomainname")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_gethostname")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_sethostname")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_gethostid")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_sethostid")	{ net_class_count++; }
			else if(get_syscall_name(line) == "sys_getpeername")	{ net_class_count++; }
			//PROCESS CONTROL
			//creation and termination
			else if(get_syscall_name(line) == "sys_exit")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_clone")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_execve")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_fork")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_vfork")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_wait")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_waitpid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_wait4")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_getpid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_getppid")	{ proc_class_count++; }
			//Signal
			else if(get_syscall_name(line) == "sys_kill")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_killpg")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_sigblock")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_sigmask")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_siggetmask")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_sigsetmask")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_signal")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_sigvec")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_sigaction")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_sigprocmask")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_sigpending")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_sigsuspend")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_sigpause")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_pause")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_sigreturn")	{ proc_class_count++; }
			//Synchronization
			else if(get_syscall_name(line) == "sys_poll")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_select")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_semctl")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_semget")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_semop")	{ proc_class_count++; }
			//User/Group ID
			else if(get_syscall_name(line) == "sys_getuid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_getgid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_getegid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_geteuid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_getresuid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_getresgid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_getsid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_getpgid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_getpgrp")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_getgroups")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setegid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_seteuid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setfsgid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setfsuid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setgid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setregid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setreuid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setresgid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setresuid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setgroups")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setsid")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setuid")	{ proc_class_count++; }
			//Resource control
			else if(get_syscall_name(line) == "sys_getrlimit")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setrlimit")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_getrusage")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_getpriority")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_nice")	{ proc_class_count++; }
			else if(get_syscall_name(line) == "sys_setpriority")	{ proc_class_count++; }
			//MEMORY
			//Virtual Memeory
			else if(get_syscall_name(line) == "sys_brk")	{ mem_class_count++; }
			else if(get_syscall_name(line) == "sys_sbrk")	{ mem_class_count++; }
			else if(get_syscall_name(line) == "sys_mlock")	{ mem_class_count++; }
			else if(get_syscall_name(line) == "sys_mlockall")	{ mem_class_count++; }
			else if(get_syscall_name(line) == "sys_munlock")	{ mem_class_count++; }
			else if(get_syscall_name(line) == "sys_munlockall")	{ mem_class_count++; }
			else if(get_syscall_name(line) == "sys_mprotect")	{ mem_class_count++; }
			else if(get_syscall_name(line) == "sys_mremap")	{ mem_class_count++; }
			else if(get_syscall_name(line) == "sys_modify_ldt")	{ mem_class_count++; }
			//Shared Memory
			else if(get_syscall_name(line) == "sys_shmctl")	{ mem_class_count++; }
			else if(get_syscall_name(line) == "sys_shmat")	{ mem_class_count++; }
			else if(get_syscall_name(line) == "sys_shmdt")	{ mem_class_count++; }
			else if(get_syscall_name(line) == "sys_shmget")	{ mem_class_count++; }
		}
	} 
	syscall_statis << APP << endl;
	syscall_statis << "File_access_class_count: " << file_class_count << endl;
	cout << "\t@attribute File_access_class_count real " << file_class_count << endl;
	syscall_statis << "Network_class_count: " << net_class_count << endl;
	cout << "\t@attribute Network_class_count real " << net_class_count << endl;
	syscall_statis << "Process_control_class_count: " << proc_class_count << endl;
	cout << "\t@attribute Process_control_class_count real " << proc_class_count << endl;
	syscall_statis << "Memory_class_count: " << mem_class_count << endl;
	cout << "\t@attribute Memory_class_count real " << mem_class_count << endl;
	syscall_statis << "Sum of the above 4 class: " << file_class_count + mem_class_count + net_class_count + proc_class_count << endl;
	syscall_statis << "Total_lines_count: " << total_line_count << endl;
	syscall_statis << "Total_lines_related_to_app_count: " << total_app_count << endl;
	syscall_statis << "Exit_syscall_count: " << exit_syscall_count << endl;
}

//build the graph data structure using the log file
graph_edge *trace_parser(char *argv[], vector<node *> &graph){
	 
	ifstream logfile(argv[1]);
	
	if(!logfile){
		cout << "Error: could not open the file!" << endl;
		exit(1);
	}
	
	struct timeval start, end;
	long mtime, seconds, useconds;
	unsigned long time = 0;
	
	string line, next_line, sys_call_name, subj_name, obj_name, file_path, file_type, root_dir;	
		
	double read_ret=0;
	double write_ret=0;
	double read_ret_sum=0;
	double write_ret_sum=0;
	double send_ret=0;
	double recv_ret=0;
	double send_ret_sum=0;
	double recv_ret_sum=0;
	
		
	//Debug paramenters
	unsigned long unknow_syscall_count = 0;
	unsigned long other_syscall_count = 0;
	unsigned long sys_execve_count = 0;
	unsigned long sys_open_count = 0;
	unsigned long sys_access_count = 0;
	unsigned long skipped_syscall_count = 0;
	//time = 0;
	unsigned long level2count = 0;
	
	unsigned long ipv4socketcount = 0;
	
	unsigned long sys_socketcount = 0;
	unsigned long socket_UNSPEC_count = 0;
	unsigned long socket_UNIX_count = 0;
	unsigned long socket_INET_count = 0;
	unsigned long socket_AX25_count = 0;
	unsigned long socket_IPX_count = 0;
	unsigned long socket_APPLETALK_count = 0;
	unsigned long socket_NETROM_count = 0;
	unsigned long socket_BRIDGE_count = 0;
	unsigned long socket_ATMPVC_count = 0;
	unsigned long socket_X25_count = 0;
	unsigned long socket_INET6_count = 0;
	unsigned long socket_ROSE_count = 0;
	unsigned long socket_DECnet_count = 0;
	unsigned long socket_NETBEUI_count = 0;
	unsigned long socket_SECURITY_count = 0;
	unsigned long socket_KEY_count = 0;
	unsigned long socket_NETLINK_count = 0; //SAME AS ROUTE
	unsigned long socket_PACKET_count = 0;
	unsigned long socket_ASH_count = 0;
	unsigned long socket_ECONET_count = 0;
	unsigned long socket_ATMSVC_count = 0;
	unsigned long socket_RDS_count = 0;
	unsigned long socket_SNA_count = 0;
	unsigned long socket_IRDA_count = 0;
	unsigned long socket_PPPOX_count = 0;
	unsigned long socket_WANPIPE_count = 0;
	unsigned long socket_LLC_count = 0;
	unsigned long socket_CAN_count = 0;
	unsigned long socket_TIPC_count = 0;
	unsigned long socket_BLUETOOTH_count = 0;
	unsigned long socket_IUCV_count = 0;
	unsigned long socket_RXRPC_count = 0;
	unsigned long socket_ISDN_count = 0;
	unsigned long socket_PHONET_count = 0;
	unsigned long socket_IEEE802154_count = 0;
	unsigned long socket_CAIF_count = 0;
	unsigned long socket_ALG_count = 0;
	unsigned long socket_NFC_count = 0;
	
	unsigned long start_line_time = 0;
	unsigned long end_line_time = 0;
	unsigned long data_point_count = 0;

	graph_edge *detection_point_graph_edge = NULL;
	
	//Debug
	ofstream debugfile;
	debugfile.open("./tmp/debugfile.dat");
	
	ofstream of_sock_type("./results/"APPNAME"/sock_type.arff");
	gettimeofday(&start, NULL);
	
	//this is necessary to initiate the start_time_stamp
	getline(logfile, line);
	start_line_time = get_time_stamp(line);
	//main while loop to build the graph 
	while (!logfile.eof()){

		time++;
		
		istream& current_line = getline(logfile, line);
		
		istream& is_read = current_line;
		istream& is_write = current_line;
		//istream& is_socket = getline(logfile, line);
		istream& is_sendto = current_line;
		istream& is_sendmsg = current_line;
		istream& is_recvmsg = current_line;
		istream& is_recvfrom = current_line;
		
		//Debug
		//remove the line.find(APP) == string::npos
		//line.find(EXIT_SYSCALL) != string::npos ||
		if(line.find(SYS_NEWSTAT) != string::npos ){
			
			skipped_syscall_count++;
			continue; //ignore the "exit" and "newstat" syscall
		}
						
		//if(have_app(line) && have_comm(line) && have_filename(line)){
		if(have_app(line) && have_filename(line)){
			sys_call_name = get_syscall_name(line);
			if(sys_call_name == "sys_execve"){
				sys_execve_count++;
				subj_name = get_subj_name(line);
				obj_name = get_obj_name(line);
				file_path = get_file_path(line);				
				file_type = PROCESS;
			}else if(sys_call_name == "sys_open"){
				sys_open_count++;
				subj_name = get_subj_name(line);
				obj_name = get_obj_name(line);
				file_path = get_file_path(line);
				
				if(obj_name.find(".so") != string::npos || 
					obj_name.find(".mo") != string::npos)
					file_type = LIBRARY;
				else 
					file_type = REG;
				
				if(subj_name != APP && obj_name == APP)
					continue;
			}else if(sys_call_name == "sys_access"){
				sys_access_count++;
				subj_name = get_subj_name(line);
				obj_name = get_obj_name(line);
				file_path = get_file_path(line);
				
				if(obj_name.find(".so") != string::npos || 
					obj_name.find(".mo") != string::npos)
					file_type = LIBRARY;
				else 
					file_type = REG;
					
				if(subj_name != APP && obj_name == APP)
					continue;
			}else
				unknow_syscall_count++;			
			// /*
			if(obj_name == "ld.so.nohwcap"){ continue;}
			else if(obj_name == "ld.so.cache"){ continue;}
			else if(obj_name == "libc.so.6"){ continue;}
			else if(obj_name == "libdl.so.2"){ continue;}
			else if(obj_name == "libstdc++.so.6"){ continue;}
			else if(obj_name == "libm.so.6"){ continue;}
			else if(obj_name == "libpthread.so.0"){ continue;}
			else if(obj_name == "libgcc_s.so.1"){ continue;}
			else if(obj_name == "filesystems"){ continue;}
			else if(obj_name == "meminfo"){ continue;}
			else if(obj_name == "urandom"){ continue;}
			// */ 
			//function block begin
			node *subj = search_graph(graph, subj_name, PROCESS, file_path); //whether should put PROCESS or file_type doesn't matter, because the subj is always created with PROCESS
			node *obj = search_graph(graph, obj_name, file_type, file_path);
			
			if(!subj){//if not exist, add new
				subj = new node(PROCESS, subj_name);
				graph.push_back(subj);
				
				subj->path = file_path;
				//cout << " 1 subj not exist: created new!!" << "::" << time << endl;
				//cout  << subj->type << "::" << subj->name << "::" << subj->path << endl;
			}


			if(!obj){
				obj = new node(file_type, obj_name);
				graph.push_back(obj);
				
				obj->path = file_path;
				//cout << " 2 obj not exist: created new!!" << "::" << time << endl;
				//cout  << obj->type << "::" << obj->name << "::" << obj->path << endl;
			}
			
			graph_edge *subj_child_edge = search_graph_edges(subj->children, obj);	
			
			if(!subj_child_edge){
				subj_child_edge = new graph_edge(subj, obj, time);
				subj->children.push_back(subj_child_edge);
				obj->parents.push_back(subj_child_edge);
				//TODO: should deal with the times and src_frequency here
			}else 
				subj_child_edge->times.push_back(time);
				
			if(subj_name != APP && obj_name == APP)
				detection_point_graph_edge = subj_child_edge;			
		
		//}else if(!have_app(line) && have_comm(line) && have_filename(line) && have_obj(graph, line)){
		}else if(!have_app(line) && have_filename(line) && have_obj(graph, line)){
			//create objects and edges
			level2count++;
			
			sys_call_name = get_syscall_name(line);
			
			if(sys_call_name == "sys_open"){
				sys_open_count++;
				subj_name = get_subj_name(line);
				obj_name = get_obj_name(line);
				file_path = get_file_path(line);
				
				if(obj_name.find(".so") != string::npos || 
					obj_name.find(".mo") != string::npos)
					file_type = LIBRARY;
				else 
					file_type = REG;
				
				if(subj_name != APP && obj_name == APP)
					continue;
			}else if(sys_call_name == "sys_access"){
				sys_access_count++;
				subj_name = get_subj_name(line);
				obj_name = get_obj_name(line);
				file_path = get_file_path(line);
				
				if(obj_name.find(".so") != string::npos || 
					obj_name.find(".mo") != string::npos)
					file_type = LIBRARY;
				else 
					file_type = REG;
					
				if(subj_name != APP && obj_name == APP)
					continue;
			}
			
			//function block begin
			node *subj = search_graph(graph, subj_name, PROCESS, file_path);
			node *obj = search_graph(graph, obj_name, file_type, file_path);
			
			if(!subj){//if not exist, add new
				subj = new node(PROCESS, subj_name);
				graph.push_back(subj);
				
				subj->path = file_path;
			}

			if(!obj){
				obj = new node(file_type, obj_name);
				graph.push_back(obj);
				
				obj->path = file_path;
			}
			
			graph_edge *subj_child_edge = search_graph_edges(subj->children, obj);	
			
			if(!subj_child_edge){
				subj_child_edge = new graph_edge(subj, obj, time);
				subj->children.push_back(subj_child_edge);
				obj->parents.push_back(subj_child_edge);
				//TODO: should deal with the times and src_frequency here
			}else 
				subj_child_edge->times.push_back(time);
				
			if(subj_name != APP && obj_name == APP)
				detection_point_graph_edge = subj_child_edge;
			//|--->function block end	

		//}else if(have_app(line) && !have_comm(line) && !have_filename(line) && have_IPv4(line)){
		}else if(have_IPv4(line)){
			//sys_call_name = get_syscall_name(line);
			ipv4socketcount++;

			subj_name = APPNAME;
			obj_name = line.substr(line.find("IPv4")-5, 2) + ":" + line.substr(line.find("IPv4") + 5, 6);
			file_path = line.substr(line.find("->")+2);
			file_type = IPv4_SOCK;
			
			//cout << "Object_type: " << file_type << "  Object_name:(df:inode) " << obj_name << "  domain:(proto) " << file_path << endl; 
			
			//function block begin
			node *subj = search_graph(graph, subj_name, PROCESS, file_path);
			node *obj = search_graph(graph, obj_name, file_type, file_path);
			
			if(!subj){//if not exist, add new
				subj = new node(PROCESS, subj_name);
				graph.push_back(subj);
				
				subj->path = file_path;
				//cout << " 1 subj not exist: created new!!" << "::" << time << endl;
				//myfile << subj->type << "::" << subj->name << "::" << subj->path << endl;
			}
						
			if(!obj){
				obj = new node(file_type, obj_name);
				graph.push_back(obj);
				
				obj->path = file_path;
				//cout << " 2 obj not exist: created new!!" << "::" << time << endl;
				//myfile << obj->type << "::" << obj->name << "::" << obj->path << endl;
				//myfile << "never executed!" << endl;
			}
			
			graph_edge *subj_child_edge = search_graph_edges(subj->children, obj);	
			
			if(!subj_child_edge){
				subj_child_edge = new graph_edge(subj, obj, time);
				subj->children.push_back(subj_child_edge);
				obj->parents.push_back(subj_child_edge);
				//TODO: should deal with the times and src_frequency here
			}else 
				subj_child_edge->times.push_back(time);
				
			if(subj_name != APP && obj_name == APP)
				detection_point_graph_edge = subj_child_edge;
			//|--->function block end
	
		}else if(have_app(line) && get_syscall_name(line) == "sys_socket"){
			sys_socketcount++;
			
			if(socket_family(line) == "1"){
				socket_UNIX_count++;
				debugfile << line << endl;
			
			}else if(socket_family(line) == "2"){
				socket_INET_count++;
			}else if(socket_family(line) == "3"){
				socket_AX25_count++;
			}else if(socket_family(line) == "4"){
				socket_IPX_count++;
			}else if(socket_family(line) == "5"){
				socket_APPLETALK_count++;
			}else if(socket_family(line) == "6"){
				socket_NETROM_count++;
			}else if(socket_family(line) == "7"){
				socket_BRIDGE_count++;
			}else if(socket_family(line) == "8"){
				socket_ATMPVC_count++;
			}else if(socket_family(line) == "9"){
				socket_X25_count++;
			}else if(socket_family(line) == "10"){
				socket_INET6_count++;
			}else if(socket_family(line) == "11"){
				socket_ROSE_count++;
			}else if(socket_family(line) == "12"){
				socket_DECnet_count++;
			}else if(socket_family(line) == "13"){
				socket_NETBEUI_count++;
			}else if(socket_family(line) == "14"){
				socket_SECURITY_count++;
			}else if(socket_family(line) == "15"){
				socket_KEY_count++;
			}else if(socket_family(line) == "16"){
				socket_NETLINK_count++;
			}else if(socket_family(line) == "17"){
				socket_PACKET_count++;
			}else if(socket_family(line) == "18"){
				socket_ASH_count++;
			}else if(socket_family(line) == "19"){
				socket_ECONET_count++;
			}else if(socket_family(line) == "20"){
				socket_ATMSVC_count++;
			}else if(socket_family(line) == "21"){
				socket_RDS_count++;
			}else if(socket_family(line) == "22"){
				socket_SNA_count++;
			}else if(socket_family(line) == "23"){
				socket_IRDA_count++;
			}else if(socket_family(line) == "24"){
				socket_PPPOX_count++;
			}else if(socket_family(line) == "25"){
				socket_WANPIPE_count++;
			}else if(socket_family(line) == "26"){
				socket_LLC_count++;
			}else if(socket_family(line) == "29"){
				socket_CAN_count++;
			}else if(socket_family(line) == "30"){
				socket_TIPC_count++;
			}else if(socket_family(line) == "31"){
				socket_BLUETOOTH_count++;
			}else if(socket_family(line) == "32"){
				socket_IUCV_count++;
			}else if(socket_family(line) == "33"){
				socket_RXRPC_count++;
			}else if(socket_family(line) == "34"){
				socket_ISDN_count++;
			}else if(socket_family(line) == "35"){
				socket_PHONET_count++;
			}else if(socket_family(line) == "36"){
				socket_IEEE802154_count++;
			}else if(socket_family(line) == "37"){
				socket_CAIF_count++;
			}else if(socket_family(line) == "38"){
				socket_ALG_count++;
			}else if(socket_family(line) == "39"){
				socket_NFC_count++;
			}
			
			//debugfile << line << endl;
		}else if(have_app(line) && get_syscall_name(line) == "sys_read"){
			//debugfile << line << endl;
			while(1){
				getline(is_read, next_line);
				if(get_syscall_name(next_line) == "exit_syscall" && cpu_id(line) == cpu_id(next_line) && have_app(next_line)){
					read_ret = get_ret(next_line);
					read_ret_sum += read_ret;
					//debugfile << next_line << endl;
					//debugfile << read_ret << endl;
					break;
				}	
			}
		}else if(have_app(line) && get_syscall_name(line) == "sys_write"){
			//debugfile << line << endl;
			while(1){
				getline(is_write, next_line);
				if(get_syscall_name(next_line) == "exit_syscall" && cpu_id(line) == cpu_id(next_line) && have_app(next_line)){
					write_ret = get_ret(next_line);
					write_ret_sum += write_ret;
					//debugfile << next_line << endl;
					//debugfile << write_ret << endl;
					break;
				}	
			}
			
		}else if(have_app(line) && get_syscall_name(line) == "sys_sendto"){
			//debugfile << line << endl;
			while(1){
				getline(is_sendto, next_line);
				if(get_syscall_name(next_line) == "exit_syscall" && cpu_id(line) == cpu_id(next_line) && have_app(next_line)){
					send_ret = get_ret(next_line);
					send_ret_sum += send_ret;
					//debugfile << next_line << endl;
					//debugfile << send_ret << endl;
					break;
				}	
			}			
		}else if(have_app(line) && get_syscall_name(line) == "sys_sendmsg"){
			//debugfile << line << endl;
			while(1){
				getline(is_sendmsg, next_line);
				if(get_syscall_name(next_line) == "exit_syscall" && cpu_id(line) == cpu_id(next_line) && have_app(next_line)){
					send_ret = get_ret(next_line);
					send_ret_sum += send_ret;
					//debugfile << next_line << endl;
					//debugfile << send_ret << endl;
					break;
				}	
			}	
		}else if(have_app(line) && get_syscall_name(line) == "sys_recvfrom"){
			//debugfile << line << endl;
			while(1){
				getline(is_recvfrom, next_line);
				if(get_syscall_name(next_line) == "exit_syscall" && cpu_id(line) == cpu_id(next_line) && have_app(next_line)){
					recv_ret = get_ret(next_line);
					recv_ret_sum += send_ret;
					//debugfile << next_line << endl;
					//debugfile << recv_ret << endl;
					break;
				}	
			}	
		}else if(have_app(line) && get_syscall_name(line) == "sys_recvmsg"){
			//debugfile << line << endl;
			while(1){
				getline(is_recvfrom, next_line);
				if(get_syscall_name(next_line) == "exit_syscall" && cpu_id(line) == cpu_id(next_line) && have_app(next_line)){
					recv_ret = get_ret(next_line);
					recv_ret_sum += send_ret;
					//debugfile << next_line << endl;
					//debugfile << recv_ret << endl;
					break;
				}	
			}
		}
		
		if(end_line_time > start_line_time + 10){
			data_point_count++; 	
		
		}
	}//while loop terminate
 
	logfile.close();
	debugfile.close();
	
	gettimeofday(&end, NULL);
    seconds  = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;	
    
	//of_sock_type << "@attribute sock_type relational" << endl;
    //Debug
    if(socket_UNSPEC_count > 0){/*cout << "\n#:: socket_UNSPEC_count: " << socket_UNSPEC_count << endl; */ of_sock_type << "\t@attribute UNSPEC real " << socket_UNSPEC_count<< endl;}
	if(socket_UNIX_count > 0)	{/*cout << "\n#:: socket_UNIX_count: " << socket_UNIX_count << endl;*/ of_sock_type << "\t@attribute UNIX real " << socket_UNIX_count << endl;}
	if(socket_INET_count > 0)	{/*cout << "\n#:: socket_INET_count: " << socket_INET_count << endl;*/ of_sock_type << "\t@attribute INET real " << socket_INET_count << endl;}
	if(socket_AX25_count > 0)	{/*cout << "\n#:: socket_AX25_count: " << socket_AX25_count << endl;*/ of_sock_type << "\t@attribute AX25 real " << socket_AX25_count << endl;}
	if(socket_IPX_count > 0)	{/*cout << "\n#:: socket_IPX_count: " << socket_IPX_count << endl;*/ of_sock_type << "\t@attribute IPX real " << socket_IPX_count << endl;}
	if(socket_APPLETALK_count > 0){/*cout << "\n#:: socket_APPLETALK_count: " << socket_APPLETALK_count << endl;*/ of_sock_type << "\t@attribute APPLETALK real " << socket_APPLETALK_count << endl;}
	if(socket_NETROM_count > 0)	{/*cout << "\n#:: socket_NETROM_count: " << socket_NETROM_count << endl;*/ of_sock_type << "\t@attribute NETROM real " << socket_NETROM_count << endl;}
	if(socket_BRIDGE_count > 0)	{/*cout << "\n#:: socket_BRIDGE_count: " << socket_BRIDGE_count << endl;*/ of_sock_type << "\t@attribute BRIDGE real " << socket_BRIDGE_count << endl;}
	if(socket_ATMPVC_count > 0)	{/*cout << "\n#:: socket_ATMPVC_count: " << socket_ATMPVC_count << endl;*/ of_sock_type << "\t@attribute ATMPVC real " << socket_ATMPVC_count<< endl;}
	if(socket_X25_count > 0)	{/*cout << "\n#:: socket_X25_count: " << socket_X25_count << endl;*/ of_sock_type << "\t@attribute X25 real " << socket_X25_count << endl;}
	if(socket_INET6_count > 0)	{/*cout << "\n#:: socket_INET6_count: " << socket_INET6_count << endl;*/ of_sock_type << "\t@attribute INET6 real " << socket_INET6_count << endl;}
	if(socket_ROSE_count > 0)	{/*cout << "\n#:: socket_ROSE_count: " << socket_ROSE_count << endl;*/ of_sock_type << "\t@attribute ROSE real " << socket_ROSE_count << endl;}
	if(socket_DECnet_count > 0)	{/*cout << "\n#:: socket_DECnet_count: " << socket_DECnet_count << endl;*/ of_sock_type << "\t@attribute DECnet real " << socket_DECnet_count << endl;}
	if(socket_NETBEUI_count > 0){/*cout << "\n#:: socket_NETBEUI_count: " << socket_NETBEUI_count << endl;*/ of_sock_type << "\t@attribute NETBEUI real " << socket_NETBEUI_count << endl;} 
	if(socket_SECURITY_count > 0){/*cout << "\n#:: socket_SECURITY_count: " << socket_SECURITY_count << endl;*/ of_sock_type << "\t@attribute SECURITY real " << socket_SECURITY_count << endl;}
	if(socket_KEY_count > 0)	{/*cout << "\n#:: socket_KEY_count: " << socket_KEY_count << endl;*/ of_sock_type << "\t@attribute KEY real " << socket_KEY_count << endl;}
	if(socket_NETLINK_count > 0){/*cout << "\n#:: socket_NETLINK_count: " << socket_NETLINK_count << endl;*/ of_sock_type << "\t@attribute NETLINK real " << socket_NETLINK_count << endl;}
	if(socket_PACKET_count > 0)	{/*cout << "\n#:: socket_PACKET_count: " << socket_PACKET_count << endl;*/ of_sock_type << "\t@attribute PACKET real " << socket_PACKET_count << endl;}
	if(socket_ASH_count > 0)	{/*cout << "\n#:: socket_ASH_count: " << socket_ASH_count << endl;*/ of_sock_type << "\t@attribute ASH real " << socket_ASH_count << endl;}
	if(socket_ECONET_count > 0)	{/*cout << "\n#:: socket_ECONET_count: " << socket_ECONET_count << endl;*/ of_sock_type << "\t@attribute ECONET real " << socket_ECONET_count << endl;}
	if(socket_ATMSVC_count > 0)	{/*cout << "\n#:: socket_ATMSVC_count: " << socket_ATMSVC_count << endl;*/ of_sock_type << "\t@attribute ATMSVC real " << socket_ATMSVC_count << endl;}
	if(socket_RDS_count > 0)	{/*cout << "\n#:: socket_RDS_count: " << socket_RDS_count << endl; */ of_sock_type << "\t@attribute RDS real " << socket_RDS_count << endl;}
	if(socket_SNA_count > 0)	{/*cout << "\n#:: socket_SNA_count: " << socket_SNA_count << endl;*/ of_sock_type << "\t@attribute SNA real " << socket_SNA_count << endl;}
	if(socket_IRDA_count > 0)	{/*cout << "\n#:: socket_IRDA_count: " << socket_IRDA_count << endl;*/ of_sock_type << "\t@attribute IRDA real " << socket_IRDA_count << endl;}
	if(socket_PPPOX_count > 0)	{/*cout << "\n#:: socket_PPPOX_count: " << socket_PPPOX_count << endl;*/ of_sock_type << "\t@attribute PPPOX real " << socket_PPPOX_count << endl;}
	if(socket_WANPIPE_count > 0){/*cout << "\n#:: socket_WANPIPE_count: " << socket_WANPIPE_count << endl;*/ of_sock_type << "\t@attribute WANPIPE real " << socket_WANPIPE_count << endl;}
	if(socket_LLC_count > 0)	{/*cout << "\n#:: socket_LLC_count: " << socket_LLC_count << endl;*/ of_sock_type << "\t@attribute LLC real " << socket_LLC_count << endl;}
	if(socket_CAN_count > 0)	{/*cout << "\n#:: socket_CAN_count: " << socket_CAN_count << endl;*/ of_sock_type << "\t@attribute CAN real " << socket_CAN_count << endl;} 
	if(socket_TIPC_count > 0)	{/*cout << "\n#:: socket_TIPC_count: " << socket_TIPC_count << endl; */ of_sock_type << "\t@attribute TIPC real " << socket_TIPC_count << endl;}
	if(socket_BLUETOOTH_count > 0){/*cout << "\n#:: socket_BLUETOOTH_count: " << socket_BLUETOOTH_count << endl;*/ of_sock_type << "\t@attribute BLUETOOTH real " << socket_BLUETOOTH_count << endl;}
	if(socket_IUCV_count > 0)	{/*cout << "\n#:: socket_IUCV_count: " << socket_IUCV_count << endl;*/  of_sock_type << "\t@attribute IUCV real " << socket_IUCV_count << endl;}
	if(socket_RXRPC_count > 0)	{/*cout << "\n#:: socket_RXRPC_count: " << socket_RXRPC_count << endl;*/ of_sock_type << "\t@attribute RXRPC real " << socket_RXRPC_count << endl;}
	if(socket_ISDN_count > 0)	{/*cout << "\n#:: socket_ISDN_count: " << socket_ISDN_count << endl;*/ of_sock_type << "\t@attribute ISDN real " << socket_ISDN_count << endl;} 
	if(socket_PHONET_count > 0)	{/*cout << "\n#:: socket_PHONET_count: " << socket_PHONET_count << endl;*/ of_sock_type << "\t@attribute PHONET real " << socket_PHONET_count << endl;}
	if(socket_IEEE802154_count > 0){/*cout << "\n#:: socket_IEEE802154_count: " << socket_IEEE802154_count << endl;*/ of_sock_type << "\t@attribute IEEE802254 real " << socket_IEEE802154_count << endl;} 
	if(socket_CAIF_count > 0)	{/*cout << "\n#:: socket_CAIF_count: " << socket_CAIF_count << endl;*/  of_sock_type << "\t@attribute CAIF real " << socket_CAIF_count << endl;} 
	if(socket_ALG_count > 0)	{/*cout << "\n#:: socket_ALG_count: " << socket_ALG_count << endl;*/  of_sock_type << "\t@attribute ALG real " << socket_ALG_count << endl;}
	if(socket_NFC_count > 0)	{/*cout << "\n#:: socket_NFC_count: " << socket_NFC_count << endl;*/ of_sock_type << "\t@attribute NFC real " << socket_NFC_count << endl;}
	
	//of_sock_type << "@end sock_type" << endl;
	of_sock_type.close();
    //output
    
    /*
	cout << "\n#:: parsing the file (while loop) is Done: \n\t" 
		 << mtime << " msecs \n\t" << endl;
	
	
   	cout << "\n#:: sys_execve_count: " << sys_execve_count << endl;
	cout << "\n#:: sys_open_count: " << sys_open_count << endl;
	cout << "\n#:: sys_access_count: " << sys_access_count << endl;
	cout << "\n#:: unknow_syscall_count: " << unknow_syscall_count << endl;
	cout << "\n#:: other_syscall_count: " << other_syscall_count << endl;
	cout << "\n#:: skipped_syscall_count: " << skipped_syscall_count << endl;
	cout << "\n#:: Total: " << sys_execve_count + sys_open_count + sys_access_count +
							unknow_syscall_count + other_syscall_count + 
							skipped_syscall_count << endl;
	cout << "\n#:: Level 2 entry: " << level2count << endl;
	cout << "\n#:: Reading socket " << ipv4socketcount << endl;
	cout << "################################################################" << endl;
	*/
	cout << "\t@attribute Total_READ numeric " << read_ret_sum/(1024*1024) << endl;
	//cout << "\t@attribute Average_READ " << read_ret_sum/(1024*1024)/TRACE_PERIOD << " MB/second" << endl;
	cout << "\t@attribute Total_WRITE numeric " << write_ret_sum/(1024*1024) << endl;
	//cout << "\t@attribute Average_WRITE " << write_ret_sum/(1024*1024)/TRACE_PERIOD << " MB/second" << endl;
	cout << "\t@attribute Socket_SENT numeric " << send_ret_sum/(1024*1024) << endl;
	cout << "\t@attribute Socket_RECEIVED numeric " << recv_ret_sum/(1024*1024) << endl;
	
	//cout << "\n" << endl;

	//cout << "time: " << time << endl;
	return detection_point_graph_edge; //this need to be assigned 
}


// this will not search through all, in fact, whenever it come across a different node, it will add a new one.
// TODOs: modify this to be correct.
node *search_graph(vector <node *> &g, string str, string type, string path){// reminder: e.g. type=PROCESS
	
	vector<node *>::iterator it;
	for(it = g.begin(); it < g.end(); it++){
		bool have_process_node = ((*it)->type == PROCESS) && ((*it)->name == str) && ((*it)->path == path);
		bool have_file_node = ((*it)->type == REG) && ((*it)->name == str)  && ((*it)->path == path);
		bool have_lib_node = ((*it)->type == LIBRARY) && ((*it)->name == str)  && ((*it)->path == path);
		
		if( have_process_node || have_file_node || have_lib_node)
			return (*it);
	}
	
	return NULL;
}


graph_edge *search_graph_edges(vector <graph_edge *> graph_edges, node* Node){
	for (unsigned int i=0; i<graph_edges.size(); i++){
		bool have_obj_node = (graph_edges[i]->dest_node->name == Node->name && 
								graph_edges[i]->dest_node->type == Node->type &&
								graph_edges[i]->dest_node->path == Node->path);
		
		if (have_obj_node)
			return graph_edges[i];
	}
	return NULL;
}


string get_syscall_name(string line){
	int index_of_start = 36; //count the index in the log file
	int index_of_end = line.find_first_of(":", 36);
	int syscall_name_len = index_of_end - index_of_start;
	return line.substr(index_of_start, syscall_name_len);
}



string get_subj_name(string line){
	int index_of_string_start = line.find("procname = \"") + 12;
	int index_of_string_end = line.find_first_of("\"", index_of_string_start);
	int subj_name_length = index_of_string_end - index_of_string_start;
	string proc_name = line.substr(index_of_string_start, subj_name_length);	
	
	return proc_name;
}
	
	
string get_file_path(string line){
	int index_of_string_start = line.find(FILENAME) + FILENAME_LEN;
	int index_of_string_end = line.find_first_of("\"", index_of_string_start);
	int file_path_len = index_of_string_end - index_of_string_start ;
	string file_path = line.substr(index_of_string_start, file_path_len);	
	
	return file_path; 
}


string get_obj_name(string line){
	string file_path =  get_file_path(line);
	string obj_name = file_path.substr(file_path.rfind("/") + 1);
	
	return obj_name;
}

string get_root_dir(string path){
	int index_of_end = path.find_first_of("/", 1);
	string root_dir = path.substr(1, index_of_end-1);
	
	return root_dir;
}


bool have_app(string line){
	if(line.find(APP) != string::npos)
		return true;
	return false;
}


bool have_comm(string line){
	if(line.find(COMM) != string::npos)
		return true;
	return false;
}


bool have_filename(string line){
	if(line.find(FILENAME) != string::npos)
		return true;
	return false;
}

bool have_IPv4(string line){
	if(line.find("IPv4") != string::npos)
		return true;
	return false;
}


//This function is search the obj that's exist match the filename of the current line
bool have_obj(vector <node *> &g, string line){
	vector <node *>::iterator it;
	
	string file_name, file_path;
	file_name = get_obj_name(line);
	file_path = get_file_path(line);
	
	for(it = g.begin(); it < g.end(); it++){
		bool have_file_node = ((*it)->name == file_name) && ((*it)->path == file_path);
		
		if(have_file_node) return true;
	}
	return false;
}


//simple hash for coloring the nodes
int h(string x, int M) {

   int i, sum;
   int xlength=x.length();
   for(sum=0, i=0; i < xlength; i++)
     sum += x[i];
   return sum % M;
}


//dgb2 function for coloring the nodes
unsigned long hash(const char *str)
{
	unsigned long hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}


int cpu_id(string line){
	int index_of_string_start = line.find("cpu_id = ") + 9;
	int index_of_string_end = line.find_first_of(" }", index_of_string_start);
	int subj_name_length = index_of_string_end - index_of_string_start;
	string id = line.substr(index_of_string_start, subj_name_length);
	int a = atoi(id.c_str());
	
	return a;
}


double get_ret(string line){
	int index_of_string_start = line.find("ret = ") + 6;
	int index_of_string_end = line.find_first_of(" }", index_of_string_start);
	int subj_name_length = index_of_string_end - index_of_string_start;
	string ret = line.substr(index_of_string_start, subj_name_length);
	int a = atoi(ret.c_str());

	if(a > 0){
		return a;
	}else 
		return 0; 
}

string socket_family(string line){
	int index_of_string_start = line.find("family = ") + 9;
	int index_of_string_end = line.find_first_of(",", index_of_string_start);
	int subj_name_length = index_of_string_end - index_of_string_start;
	string family = line.substr(index_of_string_start, subj_name_length);	
	
	return family;

}

string sub_blank4hype(string str){
	
	size_t found;
	
	found=str.find_first_of(" ");
	while(found != string::npos){
		str[found]='-';
		found=str.find_first_of(" ", found+1);
	}
	return str;
}


int get_time_stamp(string line){
	//Todo: get the time and return the integer value.
//[21:50:24.551207767] (+0.000036597) 

	int index_of_string_start = line.find(":") + 4;
	int index_of_string_end = line.find_first_of(".", index_of_string_start);
	int time_stamp_length = index_of_string_end - index_of_string_start;
	string time_stamp = line.substr(index_of_string_start, time_stamp_length);	
	
	int a = atoi(time_stamp.c_str());
	
	return a;
}


void print_nodes(vector<node *> &graph){
		cout << "print_nodes() function call output: " << endl;
	int counter = 0;
	
	vector<node *>::iterator it;
	for(it = graph.begin(); it < graph.end(); it++){
		if((*it)->type == PROCESS){
			cout << "NODE types: " << (*it)->type << " \tPROCESS name: " << (*it)->name << " \tPath: " << (*it)->path << endl;
			counter++;
		}else if((*it)->type == REG){
			//cout << "NODE types: " << (*it)->type << " \tFILE    name: " << (*it)->name << " \tPath: " << (*it)->path << endl;
			cout << "NODE types: " << (*it)->type << " \tFILE: \t\t\t\t\t\t  "  << (*it)->path << endl;
			counter++;
		}else if((*it)->type == LIBRARY){
			cout << "NODE types: " << (*it)->type << " \tLIBRARY:\t\t\t\t\t  " << (*it)->path << endl;
			counter++;
		}		
	}	
	cout << "\nSummary:: The total node in the graph " << graph.size() << endl;
}


void graph2dot(std::vector <node *> &graph){ // clustered dependency graph
	
	ofstream of1("./tmp/graph2dot_tmp.dat"); 
	ofstream of2("./results/"APPNAME"/graph2dot.dot"); 
	int hash4colorcode1;
	int hash4colorcode2;
	
	// drawing edges
	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			if(graph[i]->children[j]->dest_node->type != IPv4_SOCK && graph[i]->children[j]->dest_node->path != ""){//here we removed the case the filename = "", and make the get_root_dir() fails.
				of1 << "\t" << "\"" + graph[i]->type + ": " + graph[i]->name << "\" -> \"" << 
					graph[i]->children[j]->dest_node->type + ": " + get_root_dir(graph[i]->children[j]->dest_node->path) << "\"" << ";" << endl;
					//<< "\" [label=" << graph[i]->children[j]->times.size() << "] " << ";" << endl;
				}
		}
	} 	
	
/*	
	//coloring using the simple hash
	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			if(graph[i]->children[j]->dest_node->type != IPv4_SOCK){
				
				of1 << "\t" << "\"" + graph[i]->type + ": " + graph[i]->name + "\" [color=\"" << 
					   "0."<< h(graph[i]->name, 10000) << ", " << "0."<< h(graph[i]->name, 10000) << ", " <<"0."<< h(graph[i]->name, 10000) << "\"];" << endl
					   << "\t" << "\"" << graph[i]->children[j]->dest_node->type + ": " + get_root_dir(graph[i]->children[j]->dest_node->path) + "\" [color=\"" 
					   "0."<< h(get_root_dir(graph[i]->children[j]->dest_node->path), 100) << ", " << "0."<< h(get_root_dir(graph[i]->children[j]->dest_node->path), 100) << ", " 
					   <<"0."<< h(get_root_dir(graph[i]->children[j]->dest_node->path), 100) << "\"];" << endl;
				}
		}
	}
*/	
	//coloring using the dgb2 hash function
	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			if(graph[i]->children[j]->dest_node->type != IPv4_SOCK && graph[i]->children[j]->dest_node->path != ""){
				const char *c1 = graph[i]->name.c_str();
				hash4colorcode1 = hash(c1);
				const char *c2 = get_root_dir(graph[i]->children[j]->dest_node->path).c_str();
				hash4colorcode2 = hash(c2);
				
				of1 << "\t" << "\"" + graph[i]->type + ": " + graph[i]->name + "\" [color=\"" << 
					   "0."<< abs(hash4colorcode1)/1000000 << ", " << "0."<< abs(hash4colorcode1)/1000 - abs(hash4colorcode1)/1000000*1000 << ", " <<"0."<< abs(hash4colorcode1)- abs(hash4colorcode1)/1000*1000 << "\"];" << endl 
					   << "\t" << "\"" << graph[i]->children[j]->dest_node->type + ": " + get_root_dir(graph[i]->children[j]->dest_node->path) + "\" [color=\"" 
					   "0."<< abs(hash4colorcode2)/1000000 << ", " << "0."<< abs(hash4colorcode2)/1000 - abs(hash4colorcode2)/1000000*1000 << ", " <<"0."<< abs(hash4colorcode2)- abs(hash4colorcode2)/1000*1000 << "\"];" << endl;
					   
				}
		}
	}

	of2 << "digraph G {" << endl << "\tsize=\"8,6\"; ratio=fill; node[fontsize=24];" << endl << "\tnode [style=filled];" << endl;
	system("sort ./tmp/graph2dot_tmp.dat | uniq >> ./results/"APPNAME"/graph2dot.dot");
	system("echo } >> ./results/"APPNAME"/graph2dot.dot");
	
	of1.close();
	of2.close();
	
	return;
}


void generate_report(vector<node *> &graph){
	
	ofstream of1("./results/"APPNAME"/report.dat"); 
	of1 << "Complete Generated Report: " << endl;
	int sock_total = 0;
	int PROCESS_count = 0;
	int REG_count = 0;
	int LIB_count = 0;
	int SOCK_count = 0;
	int twoLevel_count = 0;
	int unknown = 0;
	
	ofstream of_proc("./tmp/PROCESS_tmp.dat");
	//of_proc << "Process: " << endl;
	
	ofstream of_reg("./tmp/REG_tmp.dat");
	//of_reg << "Regular files: " << endl;
	
	ofstream of_lib("./tmp/LIBRARY_tmp.dat");
	//of_lib << "Libraries: " << endl;
	
	ofstream of_sock("./tmp/IPv4_SOCK_tmp.dat");
	//of_sock << "Internet ocket: " << endl;
	
	ofstream of_2level("./tmp/TWOLEVEL_tmp.dat");
	//of_2level << "Two depth level: " << endl;
	
	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			
			
			if(graph[i]->children[j]->dest_node->type == PROCESS){
				PROCESS_count++;
				of1 << graph[i]->type + ": " + graph[i]->name << " -> " 
				<< graph[i]->children[j]->dest_node->type +": "+ 
				graph[i]->children[j]->dest_node->path << endl;
				
				//separate print
				of_proc << graph[i]->type + ": " + graph[i]->name << " -> " 
				<< graph[i]->children[j]->dest_node->type +": "+ 
				graph[i]->children[j]->dest_node->path << endl;
				
			}else if(graph[i]->name == APPNAME && graph[i]->children[j]->dest_node->type == REG){
				REG_count++;
				of1 << graph[i]->type + ": " + graph[i]->name << " -> " 
					<< graph[i]->children[j]->dest_node->type +": "+ 
					   graph[i]->children[j]->dest_node->path << endl;
				
				//separate print
				of_reg << graph[i]->type + ": " + graph[i]->name << " -> " 
					<< graph[i]->children[j]->dest_node->type +": "+ 
					   graph[i]->children[j]->dest_node->path << endl;
					   
			}else if(graph[i]->name == APPNAME && graph[i]->children[j]->dest_node->type == LIBRARY){
				LIB_count++;
				of1 << graph[i]->type + ": " + graph[i]->name << " -> " 
					<< graph[i]->children[j]->dest_node->type +": "+ 
					   graph[i]->children[j]->dest_node->path << endl;
				
				//separate print
				of_lib << graph[i]->type + ": " + graph[i]->name << " -> " 
					   << graph[i]->children[j]->dest_node->type +": "+ 
						  graph[i]->children[j]->dest_node->path << endl;
				
			}else if(graph[i]->name == APPNAME && graph[i]->children[j]->dest_node->type == IPv4_SOCK){
				SOCK_count++;
				of1 << graph[i]->type + ": " + graph[i]->name << " THOUGH " 
					<< graph[i]->children[j]->dest_node->type + "(fd:port)>" + graph[i]->children[j]->dest_node->name + " -> "+ 
					   graph[i]->children[j]->dest_node->path << endl;
				
				//separate print
				of_sock << graph[i]->type + ": " + graph[i]->name << " THOUGH " 
						<< graph[i]->children[j]->dest_node->type + "(fd:port)>" + graph[i]->children[j]->dest_node->name + " -> "+ 
						   graph[i]->children[j]->dest_node->path << endl;
				
			}else if(graph[i]->type == PROCESS && graph[i]->name != APPNAME){
				twoLevel_count++;
				of1 << graph[i]->type + ": " + graph[i]->name << " -> " 
					<< graph[i]->children[j]->dest_node->type +": "+ 
					   graph[i]->children[j]->dest_node->path << endl;
				
				//separate print
				of_2level << graph[i]->type + ": " + graph[i]->name << " -> " 
						  << graph[i]->children[j]->dest_node->type +": "+ 
							 graph[i]->children[j]->dest_node->path << endl;
	
			}else 
			
				unknown++;		
		}
	}
	
	system("sort ./tmp/PROCESS_tmp.dat | uniq > ./results/"APPNAME"/PROCESS.dat");
	system("sort ./tmp/REG_tmp.dat | uniq > ./results/"APPNAME"/REG.dat");
	system("sort ./tmp/LIBRARY_tmp.dat | uniq > ./results/"APPNAME"/LIBRARY.dat");
	system("sort ./tmp/IPv4_SOCK_tmp.dat | uniq > ./results/"APPNAME"/IPv4_SOCK.dat");
	system("sort ./tmp/TWOLEVEL_tmp.dat | uniq > ./results/"APPNAME"/TWOLEVEL.dat");
	
	of_proc.close();
	of_reg.close();
	of_lib.close();
	of_sock.close();
	of_2level.close();
	
	of1 << "\nSummary:: The PROCESS_count is: " << PROCESS_count << endl;
	of1 << "\nSummary:: The REG_count is: " << REG_count << endl;
	of1 << "\nSummary:: The LIB_count is: " << LIB_count << endl;
	of1 << "\nSummary:: The SOCK_count is: " << SOCK_count << endl;
	of1 << "\nSummary:: The twoLevel_count is: " << twoLevel_count << endl;
	of1 << "\nSummary:: The unknown is: " << unknown << endl;

	of1 << "\nSummary:: The total dependency in the graph " << PROCESS_count + REG_count + LIB_count + SOCK_count + twoLevel_count + unknown << endl;
	
	of1.close();
	
}


void graph2arff(vector <node *> &graph){ // clustered dependency graph
	//collect all the attribute
	ofstream of1("./tmp/nominaldata_tmp.arff");
	ofstream of2("./results/"APPNAME"/nominaldata.arff");
	//ofstream of2("./results/epiphany/nominaldata.arff");
	
	/*
	ofstream of_proc_tmp("./tmp/proc_tmp.arff"); 
	ofstream of_proc("./results/"APPNAME"/proc.arff"); 
	ofstream of_lib_1_tmp("./tmp/lib_1_tmp.arff"); 
	ofstream of_dep_tmp("./tmp/dep_tmp.arff"); 
	ofstream of_reg_1_tmp("./tmp/reg_1_tmp.arff"); 
	ofstream of_lib_1("./results/"APPNAME"/lib_1.arff"); 
	ofstream of_dep("./results/"APPNAME"/dep.arff"); 
	ofstream of_reg_1("./results/"APPNAME"/reg_1.arff"); 
	*/ 
	unsigned int proc_value;
	//unsigned int lib_value;
	//unsigned int reg_value;
	
//print the fork_proc
	//of_proc << "@attribute fork_proc relational" << endl;
	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			if(graph[i]->children[j]->dest_node->type != IPv4_SOCK 
				&& graph[i]->children[j]->dest_node->path != ""
				&& graph[i]->children[j]->dest_node->type == PROCESS){
				//of_proc_tmp	<< "\t@attribute proc(" + graph[i]->children[j]->dest_node->name + ") true " << endl;
				of1	<< "\t@attribute proc(" + graph[i]->children[j]->dest_node->name + ") true " << endl;
				//cout << "\t@attribute proc(" + graph[i]->children[j]->dest_node->name + ") true " << endl;
				
				proc_value++;
				}
		}
	}  
	//system("sort ./tmp/proc_tmp.arff | uniq >> ./results/"APPNAME"/proc.arff");
	//system("echo @end fork_proc >> ./results/proc.arff");
//print the lib_1
	//of_lib_1 << "@attribute lib_1 relational" << endl;
	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			if(graph[i]->children[j]->dest_node->type != IPv4_SOCK 
				&& graph[i]->children[j]->dest_node->path != ""
				&& graph[i]->children[j]->dest_node->type == LIBRARY){
				//of_lib_1_tmp << "\t@attribute lib(" + get_root_dir(graph[i]->children[j]->dest_node->path) + "/..) true " << endl;
				of1 << "\t@attribute lib(" + get_root_dir(graph[i]->children[j]->dest_node->path) + "/..) true " << endl;
				//cout << "\t@attribute lib(" + get_root_dir(graph[i]->children[j]->dest_node->path) + "/..) true " << endl;
				}
		}
	}
	//system("sort ./tmp/lib_1_tmp.arff | uniq >> ./results/"APPNAME"/lib_1.arff");
	//system("echo @end lib_1 >> ./results/lib_1.arff");	
//print the reg_1
	//of_reg_1 << "@attribute reg_1 relational" << endl;
	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			if(graph[i]->children[j]->dest_node->type != IPv4_SOCK 
				&& graph[i]->children[j]->dest_node->path != ""
				&& graph[i]->children[j]->dest_node->type == REG){
				//of_reg_1_tmp << "\t@attribute reg(" + get_root_dir(graph[i]->children[j]->dest_node->path) + "/..) true " << endl;
				of1 << "\t@attribute reg(" + get_root_dir(graph[i]->children[j]->dest_node->path) + "/..) true " << endl;
				//cout << "\t@attribute reg(" + get_root_dir(graph[i]->children[j]->dest_node->path) + "/..) true " << endl;
				}
		}
	}
	//system("sort ./tmp/reg_1_tmp.arff | uniq >> ./results/"APPNAME"/reg_1.arff");
	//system("echo @end reg_1 >> ./results/reg_1.arff");		
//print the dep
	//of_dep << "@attribute dep relational" << endl;
	for (unsigned int i=0; i<graph.size(); i++){
		for (unsigned int j=0; j<graph[i]->children.size(); j++){
			if(graph[i]->children[j]->dest_node->type != IPv4_SOCK 
				&& graph[i]->children[j]->dest_node->path != ""
				&& graph[i]->children[j]->dest_node->name != APPNAME
				&& graph[i]->name != APPNAME){
				//of_dep_tmp << "\t@attribute dep(" + graph[i]->name + ") true " << endl;
				of1 << "\t@attribute dep(" + sub_blank4hype(graph[i]->name) + ") true " << endl;
				//cout << "\t@attribute dep(" + sub_blank4hype(graph[i]->name) + ") true " << endl;
				}
		}
	}
	//system("sort ./tmp/dep_tmp.arff | uniq >> ./results/"APPNAME"/dep.arff");
	
	system("sort ./tmp/nominaldata_tmp.arff | uniq >> ./results/"APPNAME"/nominaldata.arff");
	//system("sort ./tmp/nominaldata_tmp.arff | uniq >> ./results/epiphany/nominaldata.arff");
	
	//system("echo @end dep >> ./results/dep.arff");			
	/*//print the data file for fork_proc
	of_proc << "\"";
	for(unsigned int j=0;j<proc_value;j++){
		of_proc << "true"; 
	}
	of_proc << "\"" << endl;
	
	of_proc_tmp.close();
	of_lib_1_tmp.close();
	of_dep_tmp.close();
	of_reg_1_tmp.close();
	of_proc.close();
	of_lib_1.close();
	of_dep.close();
	of_reg_1.close();
	*/
	of1.close();
	of2.close(); 
	
	return;
}


string get_attr_name(string line){
	int index_of_string_start = line.find("@attribute ")+11;
	int index_of_string_end = line.find_first_of(" ", index_of_string_start);
	int attr_name_length = index_of_string_end - index_of_string_start;
	string attr_name = line.substr(index_of_string_start, attr_name_length);	

	return attr_name;
}

string get_attr_value(string line){
	if(line.find("real") != string::npos){
		int index_of_string_start = line.find("real")+5;
		//int index_of_string_end = line.find_first_of(" ", index_of_string_start);
		//int attr_name_length = index_of_string_end - index_of_string_start;
		string attr_name = line.substr(index_of_string_start);	

		return attr_name;
	}else if(line.find("numeric") != string::npos){
		int index_of_string_start = line.find("numeric")+8;
		string attr_name = line.substr(index_of_string_start);	
		return attr_name;
		//return "\t";
	}else 
		return line;
}


string search_attribute(vector <attribute_node *> &attributes, string attr_line){
	string name = get_attr_name(attr_line);
	bool find = false;
	string line;
	for(unsigned int i=0; i<attributes.size(); i++){
		if(attributes[i]->attribute_name == name){
			line = attributes[i]->attribute_line; 
			find = true;
			break;
		}
	}
	
	if(find){
		if (line.find("real") != string::npos ||
			line.find("numeric") != string::npos){
				//cout << get_attr_value(attributes[i]->attribute_line);
				string ret_value = get_attr_value(line);
				if(ret_value.find("e-") != string::npos){
					return "0";
				}else
					return get_attr_value(line);
			}else
				//cout << "true"
				return "true";
	}else if(attr_line.find("real") != string::npos ||
			  attr_line.find("numeric") != string::npos){
				  return "0";
			}else
				return "false";
				
	//return "true"; 
}



/*
if(attributes[i]->attribute_name == name && 
			attributes[i]->attribute_line.find("real") != string::npos ||
			attributes[i]->attribute_line.find("numeric") != string::npos){
			return get_attr_value(attributes[i]->attribute_line);
		}else if(attributes[i]->attribute_name == name){
			return "true";
		}else if(name.find("real") != string::npos || name.find("numeric") != string::npos){
			return "0";
		}else
			return "flase";
*/
