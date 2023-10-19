# specify the compiler
CC=gcc
# specify options for the compiler
CFLAGS=-c -Wall

shell: shell.o util.o
	$(CC) shell.o util.o -o shell
shell.o: shell.c util.h
	$(CC) $(CFLAGS) shell.c
util.o: util.h util.c
	$(CC) $(CFLAGS) util.c
clean:
	rm -rf *o shell