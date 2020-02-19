
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "heap.h"

const int n = 10000;

bool ge(int a, int b) {
    return a >= b;
}

int rcmp(const void *pa, const void *pb) {
    int a = *(int *)pa;
    int b = *(int *)pb;
    return a < b ? 1 : (a > b ? -1 : 0);
}

void assert(bool condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "Assertion Failed: %s\n", message);
        abort();
    }
}

int main(int argc, char const *argv[]) {

    int a[n];
    
    heap *h = heap_new_empty(ge);

    for (int i = 0; i < n; i++) {
        a[i] = random() % n;
        heap_insert(h, a[i]);
    }

    heap *h2 = heap_from_array(a, n, ge);

    qsort(a, n, sizeof(int), rcmp);

    int i = 0;
    for (int i = 0; i < n; i++) {
        int top = heap_pop_top(h);
        assert(top == a[i], 
            "pop top should return next largest");

        int top2 = heap_pop_top(h2);
        assert(top2 == a[i],
            "pop top (after heapify) should return next largest");
    }

    assert(h->count == 0, "insert then pop same amount should leave heap empty");


    // TODO: test random sizes, different comparison functions

    heap_delete(h);

    return 0;
}


