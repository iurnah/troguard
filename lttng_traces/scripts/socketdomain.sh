#!/bin/bash
APP="firefox"
echo run socketdomain.sh
while [ 1 ];
do 
	lsof -i | grep "$APP" >> $APP-socket.dat;
	perl -e 'sleep(0.5)'
done
