# NAME: Zhengtong Liu
# EMAIL: ericliu2023@g.ucla.edu
# ID: 505375562

#!/bin/bash

rm -rf lab2_add.csv
rm -rf lab2_list.csv

echo "... add-none test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 20, 40, 60, 80, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num >> lab2_add.csv
    done
done
echo ""

echo "... add-m test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 20, 40, 60, 80, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num --sync=m >> lab2_add.csv
    done
done
echo ""

echo "... add-s test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 20, 40, 60, 80, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num --sync=s >> lab2_add.csv
    done
done
echo ""

echo "... add-c test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 20, 40, 60, 80, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num --sync=c >> lab2_add.csv
    done
done
echo ""

echo "... add-yield-none test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 20, 40, 80, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num --yield >> lab2_add.csv
    done
done
echo ""

echo "... add-yield-m test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 20, 40, 80, 100, 1000, 10000
    do
	    ./lab2_add --threads=$th_num --iterations=$it_num --yield --sync=m >> lab2_add.csv
    done
done
echo ""

echo "... add-yield-s test"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 20, 40, 60, 80, 100, 1000, 10000
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

echo "... list-none-none single thread"
for it_num in 1, 10, 100, 1000, 10000, 20000
do
    ./lab2_list --threads=1 --iterations=$it_num >> lab2_list.csv
done
echo ""

echo "... list-none-none mutiple threads"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 10, 100, 1000
    do
        ./lab2_list --threads=$th_num --iterations=$it_num >> lab2_list.csv
    done
done
echo ""

echo "... list-i-none"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=i >> lab2_list.csv
    done
done
echo ""

echo "... list-d-none"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=d >> lab2_list.csv
    done
done
echo ""

echo "... list-il-none"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=il >> lab2_list.csv
    done
done
echo ""

echo "... list-dl-none"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=dl >> lab2_list.csv
    done
done
echo ""

echo "... list-i-m"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=i --sync=m >> lab2_list.csv
    done
done
echo ""

echo "... list-d-m"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=d --sync=m >> lab2_list.csv
    done
done
echo ""

echo "... list-il-m"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=il --sync=m >> lab2_list.csv
    done
done
echo ""

echo "... list-dl-m"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=dl --sync=m >> lab2_list.csv
    done
done
echo ""

echo "... list-i-s"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=i --sync=s >> lab2_list.csv
    done
done
echo ""

echo "... list-d-s"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=d --sync=s >> lab2_list.csv
    done
done
echo ""

echo "... list-il-s"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=il --sync=s >> lab2_list.csv
    done
done
echo ""

echo "... list-dl-s"
for th_num in 1, 2, 4, 8, 12
do
    for it_num in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=dl --sync=s >> lab2_list.csv
    done
done
echo ""

echo "... list-none-none overcome start-up costs"
for th_num in 1, 2, 4, 8, 12, 16, 24
do
    
    ./lab2_list --threads=$th_num --iterations=1000 >> lab2_list.csv    
done
echo ""

echo "... list-none-m overcome start-up costs"
for th_num in 1, 2, 4, 8, 12, 16, 24
do
    
    ./lab2_list --threads=$th_num --iterations=1000 --sync=m >> lab2_list.csv    
done
echo ""

echo "... list-none-s overcome start-up costs"
for th_num in 1, 2, 4, 8, 12, 16, 24
do
    
    ./lab2_list --threads=$th_num --iterations=1000 --sync=s >> lab2_list.csv    
done
echo ""
