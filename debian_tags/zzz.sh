g++ facets_features.cpp -o facets_features

#INPUT: type_definition and the debian_tagdb
echo generating app_list.dat and named_app.dat
./facets_features type_def.dat debian_tagdb
echo "."
echo "."
echo "."
echo "."
echo "app_list.dat and named_app.dat has been generated"
#OUTPUT: the app_list.dat and the named_app.dat

#To read the control fields

rm ./control.dat
rm ./size_control.dat
echo "outdated control.dat and size_control.dat have been removed"
echo ">>>>>"
echo "fetch the control files"
for line in $(cat app_list.dat)
#for line in 0ad
#for line in 0ad abcde
do
	#echo -n "$line: " >> control.dat
	apt-cache show $line &> /dev/null
	if [ $? -eq 0 ]; then
		INSIZE=`apt-cache show $line | grep -m 1 "Installed-Size:" | awk '{print $2 "," }'`
		SIZE=`apt-cache show $line | grep -m 1 "^Size:" | awk '{print $2 "," }'`
		DEPENDS=`apt-cache show $line | grep -m 1 "Depends:"`
		DEPENDS=`echo $DEPENDS | awk 'BEGIN {FS = ", "}; { for(i=2;i<=NF;i++){ print "Depends::"$i "," } }'`
		DEPENDS=`echo $DEPENDS | sed 's/([^)]*)//g' | sed 's/|[^,]*,/,/g'`
		DEPENDS=`echo $DEPENDS | awk 'BEGIN {FS = " ,"}; { for(i=1;i<=NF;i++){ if (i != NF) {print $i ","} else { print $i } } };'`
		if [ -z "$INSIZE" ]; then
			INSIZE="0," 
		fi
		if [ -z "$SIZE" ]; then
			SIZE="0," 
		fi
		echo -n "$INSIZE " >> size_control.dat
		echo "$SIZE " >> size_control.dat	
		echo $DEPENDS >> control.dat
	else
		echo -n "0, " >> size_control.dat
		echo "0, " >> size_control.dat
		echo >> control.dat
	fi
	echo "$line" is fetched succesfully
done
echo "fetch control file done!!!"
echo ">>>>>"

#combine the named_app.dat and control.dat
paste -d " " named_app.dat control.dat > all_tag_control.dat
echo "tag and control combined!"
echo ">>>>>"
#OUTPUT: all_tag_control.dat file

#try to format the all_tag_control.dat in arff
./output_arff all_tag_control.dat
echo "generated header and value arff seperately"
echo ">>>>>"
#OUTPUT: arff_header.arff and arff_value.arff without size_control.dat

#combine all the arff_header, arff_value, and size_control in to a whole
paste -d " " arff_value.arff size_control.dat > arff_value_all.arff
echo "adding the installed size and size control file values as numeric attributes"
echo ">>>>>"
#OUTPUT: debian_pkg_3709.arff

#cat the header and value to get the final debian_pkg_3708.arff for weka
cat arff_header.arff arff_value_all.arff > debian_pkg_3709.arff
echo "final dataset has been generated!"
echo ">>>>>generating debian_pkg_3709.arff finished"
#OUTPUT: debian_pkg_3709.arff

