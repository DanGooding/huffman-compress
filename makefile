CC=gcc
CFLAGS=-g

out/heaptest: src/heaptest.c src/heap.c src/heap.h
	$(CC) $(CFLAGS) src/heaptest.c src/heap.c -o out/heaptest

.PHONY: clean

clean:
	rm -r out/*
