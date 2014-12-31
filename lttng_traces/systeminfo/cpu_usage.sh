#!/bin/sh

#APP="chrome"
#PIDs=$(ps aux | grep $APP | awk '{ print $2 }')
#total usage
CPULOG_1=$(cat /proc/stat | grep 'cpu ' | awk '{print $2" "$3" "$4" "$5" "$6" "$7" "$8}')
SYS_IDLE_1=$(echo $CPULOG_1 | awk '{print $4}')  
Total_1=$(echo $CPULOG_1 | awk '{print $1+$2+$3+$4+$5+$6+$7}') 

sleep 1

CPULOG_2=$(cat /proc/stat | grep 'cpu ' | awk '{print $2" "$3" "$4" "$5" "$6" "$7" "$8}')
SYS_IDLE_2=$(echo $CPULOG_2 | awk '{print $4}')  
Total_2=$(echo $CPULOG_2 | awk '{print $1+$2+$3+$4+$5+$6+$7}') 

SYS_IDLE=`expr $SYS_IDLE_2 - $SYS_IDLE_1`

Total=`expr $Total_2 - $Total_1`  
SYS_USAGE=`expr $SYS_IDLE/$Total*100 |bc -l`  
 
SYS_Rate=`expr 100-$SYS_USAGE |bc -l`  
 
Disp_SYS_Rate=`expr "scale=5; $SYS_Rate/1" |bc`  

echo CPU usage: $Disp_SYS_Rate%  
