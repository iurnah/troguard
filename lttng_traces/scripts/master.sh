#!/bin/bash
logfile=firefox-profile.txt
## command to dump firefox pid from "lttng_statedump_process_state:"
FFPID=$( awk -f firefoxpid.awk $logfile | tr -d ',' | uniq )
#echo $FFPID

export FFPID #set bash globle virables.
## command to generate accessed objects and file dependencies
awk -f ffprofile.awk $logfile | tr -d '"' | tr -d ',' | sort | uniq > ffprofile.txt

#echo $FFPID
