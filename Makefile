CC=gcc
CFLAGS=-Wall

default:
	$(CC) $(CFLAGS) -o lab3a lab3a.c
clean:
	rm lab3a
emacs:
	emacs -nw lab3a.c