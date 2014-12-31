#!/bin/bash
runtime=${1:-10s}
#Run xclock in backgound
echo $runtime
xclock&
#sleep for the specified time.
sleep $runtime
echo "All done"