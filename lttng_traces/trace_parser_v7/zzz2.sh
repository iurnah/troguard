#!/bin/bash
 
i=0 
for file in ./results/*/*.arff
do
	sed -n '15,$'p "$file" >> ./tmp/all_nominal_without_uniq.arff
	let i++
done

echo $i

sort ./tmp/all_nominal_without_uniq.arff > ./tmp/all_nominal_after_sort.arff 

uniq ./tmp/all_nominal_after_sort.arff >> ./tmp/numerical_headers.arff 

cp ./tmp/numerical_headers.arff ./datasets/attribute_declaration.arff 
