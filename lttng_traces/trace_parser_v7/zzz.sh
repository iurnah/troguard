make

export APP="chrome"
rm ./tmp/*
###########################################REPEAT PART START############################################################
./app_profile ./rawdata/"$APP"_1.dat > ./results/$APP/"$APP"numerical_1.arff
#process the the nominaldata.arff (combine the sockettype and the )
cat ./results/$APP/nominaldata.arff ./results/$APP/sock_type.arff > ./results/$APP/"$APP"nominal_1.arff
#cpu info calculations
awk '{if(NR<=11)sum+=$2; array[NR]=$2} END {for(x=1;x<=11;x++){sumsq+=((array[x]-(sum/11))^2);} print\
 "\t@attribute CPU_usage_Mean numeric " sum/11, "\n\t@attribute CPU_usage_Virance numeric " sumsq/11, "\n\t@attribute CPU_usage_Stdev numeric " sqrt(sumsq/11)}'\
 ./results/$APP/"$APP"-cpu.dat > ./results/$APP/"$APP"cpumem_1.arff
#mem info calculations
awk '{if(NR<=11)sum+=$2; array[NR]=$2} END {for(x=1;x<=11;x++){sumsq+=((array[x]-(sum/11))^2);} print\
 "\t@attribute MEM_usage_Mean numeric " sum/11, "\n\t@attribute MEM_usage_Virance numeric " sumsq/11, "\n\t@attribute MEM_usage_Stdev numeric " sqrt(sumsq/11)}'\
 ./results/$APP/"$APP"-mem.dat >> ./results/$APP/"$APP"cpumem_1.arff
#combine them all for the first 10 second data point. 
cat ./results/$APP/"$APP"numerical_1.arff ./results/$APP/"$APP"cpumem_1.arff  ./results/$APP/"$APP"nominal_1.arff > ./results/$APP/"$APP"_1.arff
#############################################REPEAT PART END###############################################################

#2#########################################################################################################################
./app_profile ./rawdata/"$APP"_2.dat > ./results/$APP/"$APP"numerical_2.arff
cat ./results/$APP/nominaldata.arff ./results/$APP/sock_type.arff > ./results/$APP/"$APP"nominal_2.arff
#cpu info calculations
awk '{if(NR>11 && NR<=21)sum+=$2; array[NR]=$2} END {for(x=12;x<=21;x++){sumsq+=((array[x]-(sum/10))^2);} print\
 "\t@attribute CPU_usage_Mean numeric " sum/10, "\n\t@attribute CPU_usage_Virance numeric " sumsq/10, "\n\t@attribute CPU_usage_Stdev numeric " sqrt(sumsq/10)}'\
 ./results/$APP/"$APP"-cpu.dat > ./results/$APP/"$APP"cpumem_2.arff
#mem info calculations
awk '{if(NR>11 && NR<=21)sum+=$2; array[NR]=$2} END {for(x=12;x<=21;x++){sumsq+=((array[x]-(sum/10))^2);} print\
 "\t@attribute MEM_usage_Mean numeric " sum/10, "\n\t@attribute MEM_usage_Virance numeric " sumsq/10, "\n\t@attribute MEM_usage_Stdev numeric " sqrt(sumsq/10)}'\
 ./results/$APP/"$APP"-mem.dat >> ./results/$APP/"$APP"cpumem_2.arff
#combine them all for the first 10 second data point. 
cat ./results/$APP/"$APP"numerical_2.arff ./results/$APP/"$APP"cpumem_2.arff ./results/$APP/"$APP"nominal_2.arff > ./results/$APP/"$APP"_2.arff

#3#########################################################################################################################
./app_profile ./rawdata/"$APP"_3.dat > ./results/$APP/"$APP"numerical_3.arff
cat ./results/$APP/nominaldata.arff ./results/$APP/sock_type.arff > ./results/$APP/"$APP"nominal_3.arff
#cpu info calculations
awk '{if(NR>21 && NR<=31)sum+=$2; array[NR]=$2} END {for(x=22;x<=31;x++){sumsq+=((array[x]-(sum/10))^2);} print\
 "\t@attribute CPU_usage_Mean numeric " sum/10, "\n\t@attribute CPU_usage_Virance numeric " sumsq/10, "\n\t@attribute CPU_usage_Stdev numeric " sqrt(sumsq/10)}'\
 ./results/$APP/"$APP"-cpu.dat > ./results/$APP/"$APP"cpumem_3.arff
#mem info calculations
awk '{if(NR>21 && NR<=31)sum+=$2; array[NR]=$2} END {for(x=22;x<=31;x++){sumsq+=((array[x]-(sum/10))^2);} print\
 "\t@attribute MEM_usage_Mean numeric " sum/10, "\n\t@attribute MEM_usage_Virance numeric " sumsq/10, "\n\t@attribute MEM_usage_Stdev numeric " sqrt(sumsq/10)}'\
 ./results/$APP/"$APP"-mem.dat >> ./results/$APP/"$APP"cpumem_3.arff
#combine them all for the first 10 second data point. 
cat ./results/$APP/"$APP"numerical_3.arff ./results/$APP/"$APP"cpumem_3.arff ./results/$APP/"$APP"nominal_3.arff > ./results/$APP/"$APP"_3.arff

# NR > $(($i-1) * 10 + 1) && NR <= $($i * 10 + 1) 
# x=$(($i-1) * 10 + 2); x<=$($i * 10 + 1)

#4#########################################################################################################################
./app_profile ./rawdata/"$APP"_4.dat > ./results/$APP/"$APP"numerical_4.arff
cat ./results/$APP/nominaldata.arff ./results/$APP/sock_type.arff > ./results/$APP/"$APP"nominal_4.arff
#cpu info calculations
awk '{if(NR>31 && NR<=41)sum+=$2; array[NR]=$2} END {for(x=32;x<=41;x++){sumsq+=((array[x]-(sum/10))^2);} print\
 "\t@attribute CPU_usage_Mean numeric " sum/10, "\n\t@attribute CPU_usage_Virance numeric " sumsq/10, "\n\t@attribute CPU_usage_Stdev numeric " sqrt(sumsq/10)}'\
 ./results/$APP/"$APP"-cpu.dat > ./results/$APP/"$APP"cpumem_4.arff
#mem info calculations
awk '{if(NR>31 && NR<=41)sum+=$2; array[NR]=$2} END {for(x=32;x<=41;x++){sumsq+=((array[x]-(sum/10))^2);} print\
 "\t@attribute MEM_usage_Mean numeric " sum/10, "\n\t@attribute MEM_usage_Virance numeric " sumsq/10, "\n\t@attribute MEM_usage_Stdev numeric " sqrt(sumsq/10)}'\
 ./results/$APP/"$APP"-mem.dat >> ./results/$APP/"$APP"cpumem_4.arff
#combine them all for the first 10 second data point. 
cat ./results/$APP/"$APP"numerical_4.arff ./results/$APP/"$APP"cpumem_4.arff ./results/$APP/"$APP"nominal_4.arff > ./results/$APP/"$APP"_4.arff

#5#########################################################################################################################
./app_profile ./rawdata/"$APP"_5.dat > ./results/$APP/"$APP"numerical_5.arff
cat ./results/$APP/nominaldata.arff ./results/$APP/sock_type.arff > ./results/$APP/"$APP"nominal_5.arff
#cpu info calculations
awk '{if(NR>41 && NR<=51)sum+=$2; array[NR]=$2} END {for(x=42;x<=51;x++){sumsq+=((array[x]-(sum/10))^2);} print\
 "\t@attribute CPU_usage_Mean numeric " sum/10, "\n\t@attribute CPU_usage_Virance numeric " sumsq/10, "\n\t@attribute CPU_usage_Stdev numeric " sqrt(sumsq/10)}'\
 ./results/$APP/"$APP"-cpu.dat > ./results/$APP/"$APP"cpumem_5.arff
#mem info calculations
awk '{if(NR>41 && NR<=51)sum+=$2; array[NR]=$2} END {for(x=42;x<=51;x++){sumsq+=((array[x]-(sum/10))^2);} print\
 "\t@attribute MEM_usage_Mean numeric " sum/10, "\n\t@attribute MEM_usage_Virance numeric " sumsq/10, "\n\t@attribute MEM_usage_Stdev numeric " sqrt(sumsq/10)}'\
 ./results/$APP/"$APP"-mem.dat >> ./results/$APP/"$APP"cpumem_5.arff
#combine them all for the first 10 second data point. 
cat ./results/$APP/"$APP"numerical_5.arff ./results/$APP/"$APP"cpumem_5.arff ./results/$APP/"$APP"nominal_5.arff > ./results/$APP/"$APP"_5.arff

#6#########################################################################################################################
./app_profile ./rawdata/"$APP"_6.dat > ./results/$APP/"$APP"numerical_6.arff
cat ./results/$APP/nominaldata.arff ./results/$APP/sock_type.arff > ./results/$APP/"$APP"nominal_6.arff
#cpu info calculations
awk '{if(NR>51 && NR<=61)sum+=$2; array[NR]=$2} END {for(x=52;x<=61;x++){sumsq+=((array[x]-(sum/10))^2);} print\
 "\t@attribute CPU_usage_Mean numeric " sum/10, "\n\t@attribute CPU_usage_Virance numeric " sumsq/10, "\n\t@attribute CPU_usage_Stdev numeric " sqrt(sumsq/10)}'\
 ./results/$APP/"$APP"-cpu.dat > ./results/$APP/"$APP"cpumem_6.arff
#mem info calculations
awk '{if(NR>51 && NR<=61)sum+=$2; array[NR]=$2} END {for(x=52;x<=61;x++){sumsq+=((array[x]-(sum/10))^2);} print\
 "\t@attribute MEM_usage_Mean numeric " sum/10, "\n\t@attribute MEM_usage_Virance numeric " sumsq/10, "\n\t@attribute MEM_usage_Stdev numeric " sqrt(sumsq/10)}'\
 ./results/$APP/"$APP"-mem.dat >> ./results/$APP/"$APP"cpumem_6.arff
#combine them all for the first 10 second data point. 
cat ./results/$APP/"$APP"numerical_6.arff ./results/$APP/"$APP"cpumem_6.arff ./results/$APP/"$APP"nominal_6.arff > ./results/$APP/"$APP"_6.arff

rm ./results/$APP/"$APP"numerical_*.arff 
rm ./results/$APP/"$APP"nominal_*.arff 
rm ./results/$APP/"$APP"cpumem_*.arff 
rm ./results/$APP/syscall_statis.dat 
rm ./results/$APP/sock_type.arff
rm ./results/$APP/nominaldata.arff


