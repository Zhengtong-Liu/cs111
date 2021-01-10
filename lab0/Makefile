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
dist: 