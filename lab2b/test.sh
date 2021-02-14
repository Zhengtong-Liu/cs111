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

