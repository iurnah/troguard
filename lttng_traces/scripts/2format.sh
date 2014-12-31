#!/bin/bash
_input="/home/rui/lttng-traces/txt/readable-firefox-lsoperation-filetred-12removed-sorted-path-rmduplic.txt"

#new='sys_access->'
#old1="sys_access: { cpu_id = 0 }, { filename = ""

`sed 's/sys_access: { cpu_id = 0 }, { filename = "/sys_access/g' $_input | \
	sed 's/sys_chmod: { cpu_id = 0 }, { filename = "/sys_chmod/g' | \
	sed 's/\//->/g' > output.txt` 
