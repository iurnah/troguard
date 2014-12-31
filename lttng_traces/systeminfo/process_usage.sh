#!/bin/bash

#APP="gcalctool"
APP="chrome"
PIDs=$(pgrep $APP)
PIDs_ARRAY=($PIDs)


PID_LEN=$(echo $PIDs | awk 'BEGIN {FS=" "} END {print NF}')

#total usage
CPULOG_1=$(cat /proc/stat | grep '^cpu ' | awk '{print $2" "$3" "$4" "$5" "$6" "$7" "$8}')
SYS_IDLE_1=$(echo $CPULOG_1 | awk '{print $4}')  
Total_1=$(echo $CPULOG_1 | awk '{print $1+$2+$3+$4+$5+$6+$7}') 

#process usage
for(( i = 0 ; i < PID_LEN ; i++ )) do

file="/proc/${PIDs_ARRAY[i]}/stat"
if [ -e $file ]
then 
PROCESSLOG_1=$(cat /proc/${PIDs_ARRAY[i]}/stat | awk '{print $14" "$15" "$16" "$17}')
PROC_TOTAL_1=$(echo $PROCESSLOG_1 | awk '{print $1+$2+$3+$4}')
PROC_TOTAL_SUM_1=`expr $PROC_TOTAL_SUM_1 + $PROC_TOTAL_1`
fi

continue

done

sleep 1

CPULOG_2=$(cat /proc/stat | grep 'cpu ' | awk '{print $2" "$3" "$4" "$5" "$6" "$7" "$8}')
SYS_IDLE_2=$(echo $CPULOG_2 | awk '{print $4}')  
Total_2=$(echo $CPULOG_2 | awk '{print $1+$2+$3+$4+$5+$6+$7}') 

#process usage

for(( i = 0 ; i < PID_LEN ; i++ )) do

file="/proc/${PIDs_ARRAY[i]}/stat"
if [ -e $file ]
then 
PROCESSLOG_2=$(cat /proc/${PIDs_ARRAY[i]}/stat | awk '{print $14" "$15" "$16" "$17}')
PROC_TOTAL_2=$(echo $PROCESSLOG_2 | awk '{print $1+$2+$3+$4}')
PROC_TOTAL_SUM_2=`expr $PROC_TOTAL_SUM_2 + $PROC_TOTAL_2`
fi

continue
done

SYS_IDLE=`expr $SYS_IDLE_2 - $SYS_IDLE_1`
Total=`expr $Total_2 - $Total_1`  
SYS_USAGE=`expr $SYS_IDLE/$Total*100 |bc -l`  
SYS_Rate=`expr 100-$SYS_USAGE |bc -l`  
Disp_SYS_Rate=`expr "scale=5; $SYS_Rate/1" |bc`  

PROC_TOTAL=`expr $PROC_TOTAL_SUM_2 - $PROC_TOTAL_SUM_1`

PROC_USAGE=`expr $PROC_TOTAL/$Total*100*8 |bc -l`

Disp_PROC_Rate=`expr "scale=5; $PROC_USAGE/1" |bc`

echo CPU usage: $Disp_SYS_Rate%      
echo $APP CPU usage: $Disp_PROC_Rate%  

