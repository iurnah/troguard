#ifndef _TRACE_PARSER_H
#define _TRACE_PARSER_H

#include "common.h"
class node;

class graph_edge{ // used as edges in dependency graph
public:
	node *src_node; 	// source node of the directed edge
	node *dest_node; 	// destination node of the directed edge
	
	float src_frequency; 	// [0,1]: how frequently the src_node uses this edge. (e.g., if frequency=1, src_node writes to only the dest_node)
	float dest_frequency; 	// [0,1]: how frequently the dest_node uses this edge. (e.g., if frequency=1, dest_node reads from only the src_node)
	
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

class node{ 	
public:
	string name, type, path;
		
	vector <graph_edge *> children; // children of the node in directed graph
	vector <graph_edge *> parents; // parents of the node in directed graph
	
	node(){};
	node(string type_a, string  name_a){
		type = type_a;
		name = name_a;
	};	
};

class dir_node{
public:
	string dir;
	vector<string *> children;
	
	dir_node(){};
	dir_node(string curr_dir){
		dir = curr_dir;
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

//graph_edge *add_node(vector <node *> graph, graph_edge *detection_point_graph_edge, string subj_name, string object_name, string file_type, string file_path, unsigned long time);
void syscall_statis(char *argv[]);

graph_edge *trace_parser(char *argv[], std::vector<node *> &graph);

node *search_graph(vector <node *> &g, string str, string type, string path);

graph_edge *search_graph_edges(vector <graph_edge *> graph_edges, node* Node);

string get_syscall_name(string line);

string get_subj_name(string line);

string get_file_path(string line);

string get_obj_name(string line);

bool have_app(string line);

bool have_comm(string line);

bool have_filename(string line);

bool have_IPv4(string line);

bool have_obj(vector <node *> &g, string line);

int h(string x, int M);

unsigned long hash(const char *str);

int cpu_id(string line);

double get_ret(string line);

string socket_family(string line);

string sub_blank4hype(string str);

int get_time_stamp(string line);

void print_nodes(vector<node *> &graph);

void graph2dot(vector <node *> &graph);

void generate_report(vector<node *> &graph);

void graph2arff(vector <node *> &graph);

void split_raw_data();

string get_attr_name(string line);

string get_attr_value(string line);

string search_attribute(vector <attribute_node *> &attributes, string line);

#endif
