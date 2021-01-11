# NAME: Zhengtong Liu
# EMAIL: ericliu2023@g.ucla.edu
# ID: 505375562

default: lab0

lab0: *.c
	gcc -Og -g3 -Wall -Wextra *.c -o $@

clean:
	rm -f *.o *.txz lab0

check: script.sh
	./script.sh 505375562

submission-files = lab0.c options.c options.h backtrace.png breakpoint.png README Makefile script.sh

lab0-505375562.tar.gz: $(submission-files)
	tar -czvf $@ $(submission-files)

dist: lab0-505375562.tar.gz
