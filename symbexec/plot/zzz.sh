#sorting the datafile based on the application name and renumber them.

cat multi_apps_normalized_percentage_userinput_symbolic.dat | awk 'BEGIN{ FS = ". " }; {$1=$2=""; print $0}' \
| sort | sed -e 's/^[ ]*//' | sed 's/\ /, /g' | awk '{printf FNR ". " $0 "\n" }' > multi_apps_normalized_percentage_userinput_symbolic_sorted.dat

