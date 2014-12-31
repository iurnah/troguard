#!/bin/bash

#APP="gcalctool"
APP="chrome"
PIDs=$(pgrep $APP)

#echo $PID1s
PIDs_ARRAY=($PIDs)

PID_LEN=$(echo $PIDs | awk 'BEGIN {FS=" "} END {print NF}')

for (( i = 0 ; i < PID_LEN ; i++ )) do
file="/proc/${PIDs_ARRAY[i]}/stat"
if [ -e $file ]
then
	MEM_USE=$(cat /proc/${PIDs_ARRAY[i]}/statm | awk '{print $6}')
	TOTAL_MEM_USE=`expr $TOTAL_MEM_USE + $MEM_USE`
fi
continue
done

TOTAL_MEM_USE_MB=`expr "scale=3; $TOTAL_MEM_USE/1024" |bc -l`

echo $APP memory usage: $TOTAL_MEM_USE_MB MB
