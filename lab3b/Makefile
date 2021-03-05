# NAME:		Bryan Tang, Zhengtong Liu
# EMAIL: 	tangtang1228@ucla.edu, ericliu2023@g.ucla.edu
# ID:    	605318712, 505375562

TARGET=lab3b

default: lab3b.py
	rm -f lab3b
	ln run_lab3b lab3b
	chmod +x lab3b

clean: 
	rm -f $(TARGET) *.tar.gz

dist:
	tar -cvzf $(TARGET)-605318712.tar.gz $(TARGET).py run_$(TARGET) Makefile README
	# tar -cvzf $(TARGET)-505375562.tar.gz $(TARGET).py run_$(TARGET) Makefile README
