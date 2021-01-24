# NAME: Zhengtong Liu
# EMAIL: ericliu2023@g.ucla.edu
# ID: 505375562

default: lab1b-client lab1b-server

lab1b-client: lab1b-client.c
	gcc -Og -g3 -Wall -Wextra -lz lab1b-client.c -o $@

lab1b-server: lab1b-server.c
	gcc -Og -g3 -Wall -Wextra -lz lab1b-server.c -o $@

clean:
	rm -rf *.o *.tar.gz lab1b-client lab1b-server *.dSYM

submission-files = *.c README Makefile

lab1b-505375562.tar.gz: $(submission-files)
	tar -czvf $@ $(submission-files)

dist: lab1b-505375562.tar.gz