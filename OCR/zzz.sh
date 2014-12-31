#pdf2txt -o Google_Chrome-CNET_Download Google_Chrome-CNET_Download.pdf
 
for dict in browser game editor graph chat player video develop calculator
do 
	map[$dict]=$(grep -c -i "$dict" ./websiteimages/output.txt)
	echo $dict ${map[$dict]}
	
done |
sort -rn -k2

