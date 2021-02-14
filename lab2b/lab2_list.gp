#! /usr/bin/gnuplot

# general plot parameters
set terminal png
set datafile separator ","

# throughput vs. number of threads for mutex and spin-lock synchronized list operations
set title "List-1: Throughput vs Number of threads"
set xlabel "Threads"
set logscale x 2
set ylabel "Number of operation per second"
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep -e 'list-none-m,[0-9]*,1000,' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/mutex' with linespoints lc rgb 'reds', \
     "< grep -e 'list-none-s,[0-9]*,1000,' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/spin-lock' with linespoints lc rgb 'green'
