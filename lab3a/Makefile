# NAME:		Bryan Tang, Zhengtong Liu
# EMAIL: 	tangtang1228@ucla.edu, ericliu2023@g.ucla.edu
# ID:    	605318712, 505375562

CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET=lab3a

default:
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET)

clean: 
	rm -rf $(TARGET) *.tar.gz

dist:
	# tar -cvzf $(TARGET)-605318712.tar.gz ext2_fs.h $(TARGET).c Makefile README
	tar -cvzf $(TARGET)-505375562.tar.gz ext2_fs.h $(TARGET).c Makefile README
