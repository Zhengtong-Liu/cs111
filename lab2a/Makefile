# NAME: Zhengtong Liu
# EMAIL: ericliu2023@g.ucla.edu
# ID: 505375562

default: lab2_add lab2_list

lab2_add: lab2_add.c
	gcc -Og -g3 -Wall -Wextra -lpthread lab2_add.c -o $@

lab2_list: lab2_list.c SortedList.h SortedList.c
	gcc -Og -g3 -Wall -Wextra -lpthread lab2_list.c SortedList.c -o $@

clean:
	rm -rf *.o *.tar.gz lab2_add lab2_list *.dSYM

submission-files = *.c README Makefile

lab2a-505375562.tar.gz: $(submission-files)
	tar -czvf $@ $(submission-files)

dist: lab2a-505375562.tar.gz