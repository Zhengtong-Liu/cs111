# NAME: Zhengtong Liu
# EMAIL: ericliu2023@g.ucla.edu
# ID: 505375562

#!/bin/bash

rm -rf lab2_list.csv

echo "... list-none-m 1000 iterations with various # of threads"
for th_num in 1, 2, 4, 8, 12, 16, 24
do
    
    ./lab2_list --threads=$th_num --iterations=1000 --sync=m >> lab2_list.csv    
done
echo ""

echo "... list-none-s 1000 iterations with various # of threads"
for th_num in 1, 2, 4, 8, 12, 16, 24
do
    
    ./lab2_list --threads=$th_num --iterations=1000 --sync=s >> lab2_list.csv    
done
echo ""

echo "... list-none-m 1000 iterations with various # of threads"
for th_num in 1, 2, 4, 8, 12, 16, 24
do
    
    ./lab2_list --threads=$th_num --iterations=1000 --sync=m >> lab2_list.csv    
done
echo ""

echo "... list-id-none 4 lists"
for th_num in 1, 2, 4, 8, 12, 16
do
    for it_num in 1, 2, 4, 8, 16
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=id --lists=4 >> lab2_list.csv  
    done  
done
echo ""

echo "... list-id-s 4 lists"
for th_num in 1, 2, 4, 8, 12, 16
do
    for it_num in 10, 20, 40, 80
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=id --lists=4 --sync=s >> lab2_list.csv  
    done  
done
echo ""

echo "... list-id-m 4 lists"
for th_num in 1, 2, 4, 8, 12, 16
do
    for it_num in 10, 20, 40, 80
    do
        ./lab2_list --threads=$th_num --iterations=$it_num --yield=id --lists=4 --sync=m >> lab2_list.csv  
    done  
done
echo ""

echo "... list-none-m 1000 iterations"
for th_num in 1, 2, 4, 8, 12
do
    for list_num in 1, 4, 8, 16
    do
        ./lab2_list --threads=$th_num --iterations=1000 --lists=$list_num --sync=m >> lab2_list.csv  
    done
done
echo ""

echo "... list-none-s 1000 iterations"
for th_num in 1, 2, 4, 8, 12
do
    for list_num in 1, 4, 8, 16
    do
        ./lab2_list --threads=$th_num --iterations=1000 --lists=$list_num --sync=s >> lab2_list.csv  
    done
done
echo ""

