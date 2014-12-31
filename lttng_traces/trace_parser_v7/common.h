#ifndef _COMMON_H
#define _COMMON_H

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
#include <cstring>

#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

#define APPNAME "chrome"
#define APP "procname = \"chrome\"" //this is to decide which application we are going to profile.
#define TRACE_PERIOD 10
#define EXIT_SYSCALL "exit_syscall:"
#define COMM "comm = \""
#define COMM_LEN 8
#define FILENAME "filename = \""
#define FILENAME_LEN 12 //length of the string "filename = \"" 
#define SYS_NEWSTAT "stat: "//exclude the sys_newstat sys_newlstat, sys_newfstat.

#define WRITE "Write"
#define READ "Read"
#define PROCESS "PROC"
#define LIBRARY "LIB"
#define IPv4_SOCK "IPv4_SOCK"


#define REG "REG"
#define SOCK_AF_UNIX "SOCK_AF_UNIX"
#define SOCK_AF_INET "SOCK_AF_INET"
#define SOCK_AF_INET6 "SOCK_AF_INET6"
#define FIFO "FIFO"
#define DIR "DIR"
#define CHR "CHR"
#define IPCMSG "IPCMSG"

using namespace std;
using std::vector;

#endif
