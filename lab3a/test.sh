#!/bin/bash
for t in SUPER GROUP BFREE IFREE INODE INDIRECT DIRENT
do
	grep $t $1 | sort > test1.csv
	grep $t $2 | sort > test2.csv
	cmp test1.csv test2.csv
	diff test1.csv test2.csv
	if [ $? -ne 0 ]; then
		echo "    $t ... OUTPUT DOES NOT MATCH"
	else
		echo "    $t ... all" `wc -l < test1.csv` "output lines match"
	fi
done