#!/bin/bash

APP="firefox"
#rm -r /root/lttng-traces/*
#lttng create rui
sleep 0.1
#lttng enable-event -a -k --syscall
sleep 0.1
#lttng start
#run $APP as a regular user(browser is not allowed to run as root)
su rui -c $APP &
#start the CPU and mem portion of the profile
#start the socket information collection
./socketdomain.sh &
#./socketip.sh
sleep 3

#kill the CPU and MEM portion of the profile
#kill the socket information collection
echo "kill the process"
ps -ef | grep 'socketdomain' | awk '{print $2}' | xargs kill -9
echo middle
ps -ef | grep '$APP' | awk '{print $2}' | xargs kill -9
echo "good!!!"
#kill firefox
#ps -ef | grep socketdomain | awk '{print $2}' | xargs kill -9

#lttng stop
#lttng destroy

#babeltrace /root/lttng-traces/$(ls /root/lttng-traces/)/ > /media/sf_DrivebyDownload/lttng-traces/txt/trash.dat
#combine the socket info and the lttng tracing
#sort $APP-socket.dat | uniq >> /media/sf_DrivebyDownload/lttng-traces/txt/trash.dat 

#ps -ef | grep [search string] | awk '{print $2}' | xargs kill -9
