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

#define WRITE "Write"
#define READ "Read"


#define PROCESS "PROCESS"
#define REG "REG"
#define SOCK_AF_UNIX "SOCK_AF_UNIX"
#define SOCK_AF_INET "SOCK_AF_INET"
#define SOCK_AF_INET6 "SOCK_AF_INET6"
#define FIFO "FIFO"
#define DIR "DIR"
#define CHR "CHR"
#define IPCMSG "IPCMSG"

// the maximum directory depth in fs tree (number of slashes) considered for the graph
#define MAX_FILE_SLASH_DEPTH 2

// these are used to get rid of corrupted log lines.
#define DEBUG_COMMAND "COMMAND:"
#define DEBUG_PROCESS "PROCESS:"
#define DEBUG_PID "PID:"
#define DEBUG_SECCON "SECCON:"
#define DEBUG_OBJTYPE "OBJTYPE:"
#define DEBUG_FD "FD:"
#define DEBUG_INODE "INODE:"
#define DEBUG_FILEPATH "FILEPATH:"
#define DEBUG_NRBUFS "NRBUFS:"
#define DEBUG_CURBUF "CURBUF:"

// these are used to measure how long a task takes.
struct timeval start, end;
long mtime, seconds, useconds;

using namespace std;
using std::vector;
