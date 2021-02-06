# NAME: Zhengtong Liu
# EMAIL: ericliu2023@g.ucla.edu
# ID: 505375562

default: lab2_add 

lab2a-add: lab2_add.c
	gcc -Og -g3 -Wall -Wextra -lpthread lab2_add.c -o $@


clean:
	rm -rf *.o *.tar.gz lab2_add *.dSYM

submission-files = *.c README Makefile

lab1b-505375562.tar.gz: $(submission-files)
	tar -czvf $@ $(submission-files)

dist: lab2a-505375562.tar.gz