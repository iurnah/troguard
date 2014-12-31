#!/bin/bash
#rm ./tmp/*
i=0 
for file in ../arff/*.arff
do
	#sed -n '15,$'p 
	awk '{print $1,$2}' "$file" >> ./tmp/all_nominal_without_uniq.arff
	let i++
done

echo $i

touch ../arff/attribute_declaration.arff

sort ./tmp/all_nominal_without_uniq.arff > ./tmp/all_nominal_after_sort.arff 

uniq ./tmp/all_nominal_after_sort.arff >> ./tmp/numerical_headers.arff 

cp ./tmp/numerical_headers.arff ../arff/attribute_declaration.arff 
