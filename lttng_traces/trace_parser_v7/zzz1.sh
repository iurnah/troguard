# in order to get this script to work, I have do a complete understanding how awk works
# come on next time.
for APP in firefox 
do
	make
	export $APP
	for i in 1 2 3 4 5 6
	do
		./app_profile ./rawdata/"$APP"_"$i".dat > ./results/$APP/"$APP"numerical_"$i".arff
		#process the the nominaldata.arff (combine the sockettype and the )
		cat ./results/$APP/nominaldata.arff ./results/$APP/sock_type.arff > ./results/$APP/"$APP"nominal_"$i".arff
		#cpu info calculations
		awk -v j=$i '{if(NR > ($j-1) * 10 + 1 && NR <= ($j * 10 + 1))sum+=$2; array[NR]=$2} END {for(x=($j-1) * 10 + 2; x<=($j * 10 + 1);x++){sumsq+=((array[x]-(sum/11))^2);} print\
		 "\t@attribute CPU_usage_Mean numeric " sum/11, "\n\t@attribute CPU_usage_Virance numeric " sumsq/11, "\n\t@attribute CPU_usage_Stdev numeric " sqrt(sumsq/11), "the j: " $j, "END"}'\
		 ./results/$APP/firefox-cpu.dat > ./results/$APP/"$APP"cpumem_"$i".arff
		#mem info calculations
		awk '{if(NR > ($i-1) * 10 + 1 && NR <= ($i * 10 + 1))sum+=$2; array[NR]=$2} END {for(x=($i-1) * 10 + 2;x<=($i * 10 + 1);x++){sumsq+=((array[x]-(sum/11))^2);} print\
		 "\t@attribute MEM_usage_Mean numeric " sum/11, "\n\t@attribute MEM_usage_Virance numeric " sumsq/11, "\n\t@attribute MEM_usage_Stdev numeric " sqrt(sumsq/11)}'\
		 ./results/$APP/firefox-mem.dat >> ./results/$APP/"$APP"cpumem_"$i".arff
		#combine them all for the first 10 second data point. 
		cat ./results/$APP/"$APP"numerical_"$i".arff ./results/$APP/"$APP"nominal_"$i".arff ./results/$APP/"$APP"cpumem_"$i".arff > ./results/$APP/"$APP"_"$i".arff
	done
done
	
