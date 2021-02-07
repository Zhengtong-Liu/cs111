# NAME: Zhengtong Liu
# EMAIL: ericliu2023@g.ucla.edu
# ID: 505375562

#!/bin/bash

rm -rf lab2_add.csv
rm -rf lab2_list.csv

echo "... add-none test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 50, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num >> lab2_add.csv
    done
done
echo ""

echo "... add-m test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 50, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num --sync=m >> lab2_add.csv
    done
done
echo ""

echo "... add-s test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 50, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num --sync=s >> lab2_add.csv
    done
done
echo ""

echo "... add-c test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 50, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num --sync=c >> lab2_add.csv
    done
done
echo ""

echo "... add-yield-none test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 50, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num --yield >> lab2_add.csv
    done
done
echo ""

echo "... add-yield-m test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 50, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num --yield --sync=m >> lab2_add.csv
    done
done
echo ""

echo "... add-yield-s test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 50, 100, 1000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num --yield --sync=s >> lab2_add.csv
    done
done
echo ""

echo "... add-yield-c test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 50, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num --yield --sync=c >> lab2_add.csv
    done
done
echo ""

