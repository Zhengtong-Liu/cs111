# NAME: ZHENGTONG LIU
# ID: 505375562
# EMAIL: ericliu2023@g.ucla.edu

default: lab4b.c
		if [ `uname -a | grep -c "armv"` -ne 0 ]; then \
		gcc -g -Wall -Wextra -lm -lmraa lab4b.c -o lab4b; \
		else \
		gcc -g -DDUMMY -lm lab4b.c -o lab4b; \
		fi

clean:
		rm -rf lab4b *.tar.gz *.dSYM

check: default
		chmod +x test.sh
		./test.sh

dist:
		tar -cvzf lab4b-505375562.tar.gz lab4b.c Makefile README test.sh


