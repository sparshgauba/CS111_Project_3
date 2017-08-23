CC=gcc
CFLAGS=-Wall

default:
	$(CC) $(CFLAGS) -o lab3a lab3a.c
clean:
	rm lab3a *.tar.gz
dist:
	tar -czvf lab3a-204600605.tar.gz lab3a.c README Makefile ext2_fs.h
