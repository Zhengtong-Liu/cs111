# NAME: ZHENGTONG LIU
# ID: 505375562
# EMAIL: ericliu2023@g.ucla.edu

default:
    gcc -g -Wall -Wextra -lm -lmraa lab4b.c -o lab4b

clean:
	rm -rf lab4b *.tar.gz

check: default
	chmod +x test.sh
	./test.sh

dist:
	tar -cvzf lab4b-505375562.tar.gz lab4b.c Makefile README 


