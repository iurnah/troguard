#!/bin/bash

#runtime=${1:-1m}

rm -r /root/lttng-traces/*
lttng create rui-session
lttng enable-event -a -k --syscall

#lttng enable-event sched_wakeup,sched_wakeup_new,sched_switch,sched_migrate_task,\
sched_process_free,sched_process_exit,sched_wait_task,sched_process_wait,sched_process_fork,\
sched_stat_wait,sched_stat_sleep,sched_stat_iowait,sched_stat_runtime,sched_pi_setprio,\
lttng_statedump_start,lttng_statedump_end,lttng_statedump_process_state,lttng_statedump_file_descriptor,\
lttng_statedump_vm_map,lttng_statedump_network_interface,lttng_statedump_interrupt -k

lttng start

firefox 
./dump-socket-domain.sh
kill $!
#sleep $runtime
lttng stop
lttng destroy rui-session
echo 'lttng has stopped'

babeltrace /root/lttng-traces/$(ls /root/lttng-traces/)/ > trash.txt
geany trash.txt &
