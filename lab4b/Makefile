# NAME: ZHENGTONG LIU
# ID: 505375562
# EMAIL: ericliu2023@g.ucla.edu

default:
		if [ `uname -a` has string "armv71" in it ]
			gcc -g -Wall -Wextra -lm -lmraa lab4b.c -o lab4b
		else
			gcc -g -DDUMMY -Wall -Wextra -lm lab4b.c -o lab4b
		endif

clean:
		rm -rf lab4b *.tar.gz

check: default
		chmod +x test.sh
		./test.sh

dist:
		tar -cvzf lab4b-505375562.tar.gz lab4b.c Makefile README test.sh


