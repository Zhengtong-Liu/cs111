# NAME: Zhengtong Liu
# EMAIL: ericliu2023@g.ucla.edu
# ID: 505375562

build: lab2_add lab2_list

lab2_add: lab2_add.c
	gcc -Og -g3 -Wall -Wextra -lpthread lab2_add.c -o $@

lab2_list: lab2_list.c SortedList.h SortedList.c
	gcc -Og -g3 -Wall -Wextra -lpthread lab2_list.c SortedList.c -o $@

clean:
	rm -rf *.o *.tar.gz lab2_add lab2_list *.dSYM

tests: build
	chmod +x test.sh
	./test.sh

graphs: tests
	gnuplot ./lab2_add.gp

submission-files = *.c *.h README Makefile *.csv *.png test.sh *.gp

dist: build graphs $(submission-files)
	tar -czvf lab2a-505375562.tar.gz $(submission-files)