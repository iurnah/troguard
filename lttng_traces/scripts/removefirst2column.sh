#!/bin/bash
_input="/home/rui/lttng-traces/txt/readable-firefox-lsoperations-filtered.txt"
#readable-firefox-lsoperation-filetred-12removed-sorted-path-rmduplic.txt

`cat $_input | awk '{ print substr($0, index($0,$3))}' > readable-firefox-lsoperation-filetred-12removed.txt`
