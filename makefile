CC=gcc
CFLAGS=-g

out/bitstringtest: src/bitstringtest.c src/bitstring.c src/bitstring.h
	$(CC) $(CFLAGS) src/bitstringtest.c src/bitstring.c -o out/bitstringtest

out/heaptest: src/heaptest.c src/heap.c src/heap.h
	$(CC) $(CFLAGS) src/heaptest.c src/heap.c -o out/heaptest

.PHONY: clean

clean:
	rm -r out/*
