
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "heap.h"
#include "assert.h"

const int n = 10;

typedef struct {
    int key;
    char value;
} element;

bool ge(const void *a, const void *b) {
    return ((element *)a)->key >= ((element *)b)->key;
}

int rcmp(const void *pa, const void *pb) {
    element *a = *(element **) pa;
    element *b = *(element **) pb;
    return a->key > b->key ? -1 : (a->key < b->key ? 1 : 0);
}

element *random_element() {
    element *e = malloc(sizeof(element));
    e->key = random() % n;
    e->value = (double)random();
    return e;
}

bool key_equals(element *a, element *b) {
    return a->key == b->key;
}

int main(int argc, char const *argv[]) {
    srand(42);

    element **a = malloc(sizeof(element *) * n);
    
    heap *h = heap_new_empty(ge);

    for (int i = 0; i < n; i++) {
        a[i] = random_element();
        heap_insert(h, (void *)(a[i]));
    }

    heap *h2 = heap_from_array((void **)a, n, ge);
    assert(h->count == n, 
    "result from heap_from_array should have correct number of elements");

    qsort(a, n, sizeof(element *), rcmp);

    for (int i = 1; i < n; i++) {
        element *prev = a[i - 1];
        element *curr = a[i];

        assert(prev->key >= curr->key, 
            "a[] should be descending");
    }

    for (int i = 0; i < n; i++) {
        void *top = heap_pop_top(h);
        assert(key_equals(top, (void *)a[i]), 
            "pop top should return next largest");

        void *top2 = heap_pop_top(h2);
        assert(key_equals(top2, (void *)a[i]),
            "pop top (after heapify) should return next largest");
    }

    assert(h->count == 0, "insert then pop same amount should leave heap empty");


    // TODO: test random sizes, different comparison functions

    // now empty
    heap_delete_only(h);
    heap_delete_only(h2);
    for (int i = 0; i < n; i++) {
        free(a[i]);
    }
    free(a);

    return 0;
}


