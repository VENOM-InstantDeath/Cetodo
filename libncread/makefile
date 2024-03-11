CC=gcc
CFLAGS=
LIBS=-lasan -lncurses -lm

x: test.c nc.c
	$(CC) $(CFLAGS) $(LIBS) test.c nc.c ../libvector/vector.c -o x -g
