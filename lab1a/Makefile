# NAME: Zhengtong Liu
# EMAIL: ericliu2023@g.ucla.edu
# ID: 505375562

default: lab1a

lab1a: lab1a.c
	gcc -Og -g3 -Wall -Wextra lab1a.c -o $@

clean:
	rm -rf *.o *.tar.gz lab1a *.dSYM

submission-files = lab1a.c README Makefile

lab1a-505375562.tar.gz: $(submission-files)
	tar -czvf $@ $(submission-files)

dist: lab1a-505375562.tar.gz