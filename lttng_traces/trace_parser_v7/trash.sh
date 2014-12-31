
j=2
echo $(($j * 10 + 1))
echo $((($j-1) * 10 + 1))
for i in 1 2 3 4 5 6
do
	awk '{ print $1 $2 }' ./results/firefox/firefox-cpu.dat 
done
