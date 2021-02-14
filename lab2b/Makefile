# NAME: Zhengtong Liu
# EMAIL: ericliu2023@g.ucla.edu
# ID: 505375562

default:
	gcc -g -Wall -Wextra -lpthread -lprofiler lab2_list.c SortedList.c -o lab2_list

clean:
	rm -rf *.o *.tar.gz lab2_list *.dSYM

tests: default
	chmod +x test.sh
	-./test.sh

profile: default
	rm -f ./raw.gperf profile.out
	LD_PRELOAD=~/lib/libprofiler.so
	CPUPROFILE=./raw.gperf ./lab2_list --thread=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=thread_worker ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

graphs: tests
	gnuplot ./lab2_list.gp

submission-files = *.c *.h README Makefile *.csv *.png test.sh *.gp

dist: graphs profile
	tar -czvf lab2a-505375562.tar.gz $(submission-files)
