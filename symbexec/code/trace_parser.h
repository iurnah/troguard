#ifndef _TRACE_PARSER_H
#define _TRACE_PARSER_H

#include "common.h"
class node;
class lttng_read;
class lttng_write;

class graph_edge{ // used as edges in dependency graph
public:
	node *src_node; 	// source node of the directed edge
	node *dest_node; 	// destination node of the directed edge

	string edge_type, bytes;
	double timestamp;
	//float src_frequency; 	// [0,1]: how frequently the src_node uses this edge. (e.g., if frequency=1, src_node writes to only the dest_node)
	//float dest_frequency; 	// [0,1]: how frequently the dest_node uses this edge. (e.g., if frequency=1, dest_node reads from only the src_node)

	vector <unsigned long> times; // time series which this (data flow) edge was taken

	graph_edge(){}
	graph_edge(node *src, node *dst){
		src_node = src;
		dest_node = dst;
	};
	//graph_edge(node *src, node *dst, unsigned long t){
	graph_edge(node *src, node *dst, double current_time, string RW_bytes){
		src_node = src;
		dest_node = dst;
		timestamp = current_time;
		bytes = RW_bytes;
	};
};

class node{
public:
    double timestamp, timestamp_end;
	string type, name, path, read_bytes, write_bytes, fd;
	string family;
	int count; //this is for socket family count;
	long int send_data, recv_data;
    //might need map also for searching
	vector <graph_edge *> children; // children of the node in directed graph
	vector <graph_edge *> parents; // parents of the node in directed graph

	node(){};
    //proc constructor
	node(double stamp_a, string fd_a, string type_a, string name_a){
		timestamp = stamp_a;
		type = type_a;
		name = name_a;
		fd = fd_a;
		path = "";
	};
    //read/write constructor
    node(double stamp_a, string fd_a,
		 string type_a, string name_a,
		 string path_a, string read_bytes_a, string write_bytes_a)
	{
        timestamp = stamp_a;
        type = type_a;
        name = name_a;
        path = path_a;
        //data_access = data_access_a;
        read_bytes = read_bytes_a;
        write_bytes = write_bytes_a;
        fd = fd_a;
	};
    //socket constructor
    node(double stamp_a, double stamp_end,
		 string name_a, string type_a,
		 string family_a, string fd_a,
		 int send_a, int recv_a)
	{
        timestamp = stamp_a;
        timestamp_end = stamp_end;
        name = name_a;
        type = type_a;
        family = family_a;
        fd = fd_a;
        send_data = send_a;
        recv_data = recv_a;
        count = 1;
	};


};

class lttng_open{ //also for creat
public:
	double timestamp;
	string cpuid, procname, fd, filepath, filename, retvalue;
// TODO (rui#1#): To initialize all the default constructors to be null instands of unknow value.?
	lttng_open(){};
	lttng_open(
		double current_time,
		string get_cpuid,
		string get_procname,
		string get_filepath,
		string get_filename,
		string get_fd,
		string get_ret)
	{
		timestamp = current_time;
		cpuid = get_cpuid;
		procname = get_procname;
		filepath = get_filepath;
		filename = get_filename;
		fd = get_fd;
		retvalue = get_ret;
	};

};

class lttng_close{ //also for creat
public:
	double timestamp;
	string cpuid, procname, fd, retvalue;

	lttng_close(){};
	lttng_close(
		double current_time,
		string get_cpuid,
		string get_procname,
		string get_fd,
		string get_ret)
	{
		timestamp = current_time;
		cpuid = get_cpuid;
		procname = get_procname;
		fd = get_fd;
		retvalue = get_ret;
	};

};

class lttng_read{
public:
	double timestamp;
	string cpuid, procname, fd, filepath, filename, type, retvalue, reg_sock_flag;

	lttng_read(){};
	lttng_read(
		double current_time,
		string get_cpuid,
		string get_procname,
		string get_fd,
		string get_retvalue)
	{
		timestamp = current_time;
		cpuid = get_cpuid;
		procname = get_procname;
		fd = get_fd;
		retvalue = get_retvalue;
		type = "r";
		filename = "";
		filepath = "";
	};
};

class lttng_write{
public:
	double timestamp;
	string cpuid, procname, fd, filepath, filename, type, retvalue;

	lttng_write(){};
	lttng_write(
		double current_time,
		string get_cpuid,
		string get_procname,
		string get_fd,
		string get_retvalue)
	{
		timestamp = current_time;
		cpuid = get_cpuid;
		procname = get_procname;
		fd = get_fd;
		retvalue = get_retvalue;

		type = "w";
        filename = "";
		filepath = "";
	};
};

class lttng_fcntl{
public:
	double timestamp;
	string cpuid, procname, fd, arg, type, retvalue;
    int cmd;

	lttng_fcntl(){};
	lttng_fcntl(
		double current_time,
		string get_cpuid,
		string get_procname,
		string get_fd,
		int get_cmd,
		string get_arg,
		string get_retvalue)
	{
		timestamp = current_time;
		cpuid = get_cpuid;
		procname = get_procname;
		fd = get_fd;
		cmd = get_cmd;
		arg = get_arg;
		retvalue = get_retvalue;
		type = "fcntl";
	};
};

class lttng_socket{
public:
	double timestamp, timestamp_end;
	string cpuid, procname, family, fd, type, retvalue;
	long int total_sent, total_recved;

	lttng_socket(){};
	lttng_socket(
		double current_time,
		double close_time,
		string get_cpuid,
		string get_procname,
		string get_family,
		string get_fd,
		string get_retvalue)
	{
		timestamp = current_time;
		timestamp_end = close_time;
		cpuid = get_cpuid;
		procname = get_procname;
        family = get_family;
        fd = get_fd;
		retvalue = get_retvalue;
		total_sent = 0;
		total_recved = 0;
		type = "sock";
	};
};

class lttng_socket_send{
public:
	double timestamp;
	string cpuid, procname, fd, type, retvalue;


	lttng_socket_send(){};
	lttng_socket_send(
		double current_time,
		string get_cpuid,
		string get_procname,
		string get_fd,
		string get_retvalue)
	{
		timestamp = current_time;
		cpuid = get_cpuid;
		procname = get_procname;
        fd = get_fd;
		retvalue = get_retvalue;
		type = "sock_send";
	};
};

class lttng_socket_recv{
public:
	double timestamp;
	string cpuid, procname, fd, type, retvalue;

	lttng_socket_recv(){};
	lttng_socket_recv(
		double current_time,
		string get_cpuid,
		string get_procname,
		string get_fd,
		string get_retvalue)
	{
		timestamp = current_time;
		cpuid = get_cpuid;
		procname = get_procname;
        fd = get_fd;
		retvalue = get_retvalue;
		type = "sock_recv";
	};
};

class attribute_node{
public:
	string attribute_name, attribute_line, appclass, datapoint;
	attribute_node(){};
	attribute_node(string name, string line){
		attribute_name = name;
		attribute_line = line;	
	};		
};


//rui__graph_edge *trace_parser(char *argv[], vector<node *> &graph);
graph_edge *trace_parser(char *argv[], multimap <string, node *> &graph);

//rui__graph_edge *add_read(vector<node *> &graph, lttng_read * current_read);
graph_edge *add_read(multimap <string, node *> &graph, lttng_read * current_read);

//rui__graph_edge *add_write(vector<node *> &graph, lttng_write * current_write);
graph_edge *add_write(multimap <string, node *> &graph, lttng_write * current_write);

//rui__graph_edge *add_socket(vector<node *> &graph, lttng_socket * current_socket);
graph_edge *add_socket(multimap <string, node *> &graph, lttng_socket * current_socket);

void add_sock_sends(multimap <string, node *> &graph, lttng_socket_send * current_sock_sends);

void add_sock_recvs(multimap <string, node *> &graph, lttng_socket_recv * current_sock_recvs);

//rui__node *search_graph(vector <node *> &g, string type, string name, string path);
node *search_graph(multimap <string, node *> &g, string type, string name, string path);

//rui__void clear_graph(vector<node *> &graph);
void clear_graph(multimap <string, node *> &graph);

//rui__void graph_traversal(vector<node *> &graph);
void graph_traversal(multimap <string, node *> &graph);

//rui__void graph_traversal_depend(vector<node *> &graph);
void graph_traversal_depend(multimap <string, node *> &graph);

void print_depend(multimap <string, node *> &graph);

//MOD: map<string, string> get_attribute(multimap<string, node *> &graph, char *argv[], double point);
map<string, string> get_attribute(multimap<string, node *> &graph, char *argv[]);

//map <string, string> get_attr_grp0(char *argv[], double low, double high);

//map <string, string> get_attr_grp1(multimap<string, node *> &graph, double low, double high);

//map <string, string> get_attr_grp2(multimap<string, node *> &graph, double low, double high);

map <string, string> get_attr_grp0(char *argv[]);

map <string, string> get_attr_grp1(multimap<string, node *> &graph);

map <string, string> get_attr_grp2(multimap<string, node *> &graph);

//map <string, string> get_attr_grp3(char *argv[], string period);

string get_sample_period(string line);

string get_ip(string line);

string get_port(string line);

double get_time_stamp(string line);

string get_cpu_id(string line);

string get_proc_name(string line);

string get_file_path(string line);

string get_file_name(string line);

string get_fd(string line);

string get_ret(string line);

int get_fcntl_cmd(string line);

string get_fcntl_arg(string line);

string get_socket_family(string line);

string get_root_dir(string path);

string doubleToString(double val);

string longToString(long int val);

string get_sock_type(string family);

string intToString(unsigned int val);

bool have_app(string line);

string get_syscall_name(string line);

string get_attr_name(string line);

string search_attribute(vector <attribute_node *> &attributes, string attr_line);

#endif

