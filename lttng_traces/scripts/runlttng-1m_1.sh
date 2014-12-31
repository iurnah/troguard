#!/bin/bash
runtime=${1:-1m}
`lttng enable-event -a -k --syscall`

`lttng enable-event sched_wakeup,sched_wakeup_new,sched_switch,sched_migrate_task,\
sched_process_free,sched_process_exit,sched_wait_task,sched_process_wait,sched_process_fork,\
sched_stat_wait,sched_stat_sleep,sched_stat_iowait,sched_stat_runtime,sched_pi_setprio,\
lttng_statedump_start,lttng_statedump_end,lttng_statedump_process_state,lttng_statedump_file_descriptor,\
lttng_statedump_vm_map,lttng_statedump_network_interface,lttng_statedump_interrupt -k`

echo "lttng enable-event all syscalls and sched_wakeup,sched_wakeup_new,sched_switch,sched_migrate_task, \
sched_process_free,sched_process_exit,sched_wait_task,sched_process_wait,sched_process_fork,\
sched_stat_wait,sched_stat_sleep,sched_stat_iowait,sched_stat_runtime,sched_pi_setprio, \
lttng_statedump_start,lttng_statedump_end,lttng_statedump_process_state,lttng_statedump_file_descriptor, \
lttng_statedump_vm_map,lttng_statedump_network_interface,lttng_statedump_interrupt"

`lttng start`
echo "lttng has been started and will run for 1 minutes"
sleep $runtime
`lttng stop`
echo "lttng has stopped"
