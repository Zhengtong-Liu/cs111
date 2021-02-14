#! /usr/bin/gnuplot

# general plot parameters
set terminal png
set datafile separator ","

# throughput vs. number of threads for mutex and spin-lock synchronized list operations
set title "List-1: Throughput vs Number of threads"
set xlabel "Threads"
set logscale x 2
set ylabel "Number of operations per second"
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/mutex' with linespoints lc rgb 'reds', \
     "< grep -e 'list-none-s,[0-9]*,1000,1' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/spin-lock' with linespoints lc rgb 'green'


# mean time per mutex wait and mean time per operation for mutex-synchronized list operations
set title "List-2: Average wait-for-mutex time and average time per operation vs Number of threads"
set xlabel "Threads"
set logscale x 2
set ylabel "Average time (ns)"
set logscale y 10
set output 'lab2b_2.png'

plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1' lab2_list.csv" using ($2):($7) \
	title 'average time per operation' with linespoints lc rgb 'reds', \
     "< grep -e 'list-none-m,[0-9]*,1000,1' lab2_list.csv" using ($2):($8) \
	title 'average wait-for-mutex time' with linespoints lc rgb 'green'



set title "List-3: Sucessful Iterations vs. threads for each synchronization method"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep -e 'list-id-none,[0-9]*,[0-9]*,4' lab2_list.csv" using ($2):($3) \
	title 'yield=id sync=none' with points lc rgb 'red', \
     "< grep -e 'list-id-m [0-9]*,[0-9]*,4' lab2_list.csv" using ($2):($3) \
	title 'yield=id sync=m' with points lc rgb 'green', \
     "< grep -e 'list-id-s [0-9]*,[0-9]*,4' lab2_list.csv" using ($2):($3) \
	title 'yield=d sync=s' with points lc rgb 'blue'


set title "List-4: throughput vs. number of threads for mutex synchronized partitioned lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Number of operation per second"
set logscale y 10
set output 'lab2b_4.png'
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=1' with linespoints lc rgb 'green', \
     "< grep -e 'list-none-m,[0-9]*,1000,4' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=4' with linespoints lc rgb 'red', \
     "< grep -e 'list-none-m,[0-9]*,1000,8' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=8' with linespoints lc rgb 'orange', \
     "< grep -e 'list-none-m,[0-9]*,1000,16' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=16' with linespoints lc rgb 'blue'


set title "List-5: throughput vs. number of threads for spin-lock-synchronized partitioned lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Number of operation per second"
set logscale y 10
set output 'lab2b_5.png'
plot \
     "< grep -e 'list-none-s,[0-9]*,1000,1' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=1' with linespoints lc rgb 'green', \
     "< grep -e 'list-none-s,[0-9]*,1000,4' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=4' with linespoints lc rgb 'red', \
     "< grep -e 'list-none-s,[0-9]*,1000,8' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=8' with linespoints lc rgb 'orange', \
     "< grep -e 'list-none-s,[0-9]*,1000,16' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=16' with linespoints lc rgb 'blue'