CC=gcc
LIBS=-lasan -lncurses -lm -ljson-c
CFLAGS=-I/usr/include/json-c

cetodo: main.c libvector/vector.o libncread/ncread.c
	$(CC) $(LIBS) $(CFLAGS) main.c libvector/vector.o libncread/ncread.c -o cetodo -g
