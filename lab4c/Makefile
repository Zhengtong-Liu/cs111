# NAME: ZHENGTONG LIU
# ID: 505375562
# EMAIL: ericliu2023@g.ucla.edu

default: lab4c_tcp lab4c_tls

lab4c_tcp: lab4c_tcp.c
		if [ `uname -a | grep -c "armv"` -ne 0 ]; then \
		gcc -g -Wall -Wextra -lm -lmraa lab4c_tcp.c -o lab4c_tcp; \
		else \
		gcc -g -DDUMMY -lm lab4c_tcp.c -o lab4c_tcp; \
		fi

lab4c_tls: lab4c_tls.c
		if [ `uname -a | grep -c "armv"` -ne 0 ]; then \
		gcc -g -Wall -Wextra -lcrypto -lssl -lm -lmraa lab4c_tls.c -o lab4c_tls; \
		else \
		gcc -g -DDUMMY -lcrypto -lssl -lm lab4c_tls.c -o lab4c_tls; \
		fi

clean:
		rm -rf lab4c_tcp lab4c_tls *.tar.gz *.dSYM

dist:
		tar -cvzf lab4c-505375562.tar.gz *.c Makefile README


