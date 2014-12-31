#!/bin/bash

#make -f ./Makefile-replaygen

rm /home/rui/DrivebyDownload/symbexec/test-cases/replay/*

./replay-gen

FILES=/home/rui/DrivebyDownload/symbexec/test-cases/replay/*
for f in $FILES
do
	echo "Processing $f"
	sort $f | uniq > $f.new
	cp $f.new $f
	rm $f.new
	chmod +x $f
	echo $f
done
