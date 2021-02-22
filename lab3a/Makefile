# Name: Zhengtong Liu
# Email: ericliu2023@g.ucla.edu
# UID: 505375562

CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET=lab3a

default:
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET)

clean: 
	rm -rf $(TARGET) *.tar.gz *.dSYM

dist:
	tar -cvzf $(TARGET)-505375562.tar.gz ext2_fs.h $(TARGET).c Makefile README
