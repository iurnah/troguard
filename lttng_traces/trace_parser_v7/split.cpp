#include "common.h"
#include "trace_parser.h"

#define APPNAME "firefox"
int main(){
	unsigned long start_line_time;
	unsigned long end_line_time;
	string line;
	bool flag = false;
	//system("mkdir ~/DrivebyDownload/lttng-traces/trace_parser_v2/rawdata/"APPNAME);
	ifstream original_dataset("./rawdata/"APPNAME".dat");
	
	getline(original_dataset, line);
	
	start_line_time = get_time_stamp(line);
	end_line_time = start_line_time;
	
	ofstream of1("./rawdata/"APPNAME"_1.dat");
	ofstream of2("./rawdata/"APPNAME"_2.dat");
	ofstream of3("./rawdata/"APPNAME"_3.dat");
	ofstream of4("./rawdata/"APPNAME"_4.dat");			
	ofstream of5("./rawdata/"APPNAME"_5.dat");
	ofstream of6("./rawdata/"APPNAME"_6.dat");
	cout << "partitioning the rawdata file ...";
	while(!original_dataset.eof()){
		getline(original_dataset, line);
		
		if(flag == true && end_line_time - start_line_time == 0){
			break;
		}
		
		if(line.find("cpu_id =") == string::npos) 
			continue;
			
		if(end_line_time - start_line_time < 10){
			of1 << line << endl;
			
		}else if(end_line_time - start_line_time < 20){
			of2 << line << endl;
			flag = true;
		}else if(end_line_time - start_line_time < 30){
			of3 << line << endl;
		}else if(end_line_time - start_line_time < 40){
			of4 << line << endl;
		}else if(end_line_time - start_line_time < 50){
			of5 << line << endl;
		}else if(end_line_time - start_line_time < 60){
			of6 << line << endl;
		}

		end_line_time = get_time_stamp(line);
		if(end_line_time < start_line_time){
			end_line_time = 60 + end_line_time;

		}
		
		
	}
	cout << endl << "Partitioning Done!";
}
