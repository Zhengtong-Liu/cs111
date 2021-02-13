# NAME: Zhengtong Liu
# EMAIL: ericliu2023@g.ucla.edu
# ID: 505375562

build: lab2_list

lab2_list: lab2_list.c SortedList.h SortedList.c
	gcc -Og -g3 -Wall -Wextra -lpthread lab2_list.c SortedList.c -o $@

clean:
	rm -rf *.o *.tar.gz lab2_list *.dSYM

tests: build
	chmod +x test.sh
	-./test.sh

profile: lab2_list
	LD_PRELOAD = /lib64/libprofiler.so
	CPUPROFILE = ./raw.gperf ./lab2_list --thread=12 --iterations=1000 --sync=s
	/bin/pprof --text ./lab2_list ./raw.gperf
	/bin/pprof --list=thread_worker ./lab2_list ./raw.gperf

graphs: tests
	gnuplot ./lab2_list.gp

submission-files = *.c *.h README Makefile *.csv *.png test.sh *.gp

dist: graphs
	tar -czvf lab2a-505375562.tar.gz $(submission-files)
