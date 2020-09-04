#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>

typedef bool (*element_cmp) (const void *, const void *);

// TODO hide internals by making heap* alias of void*
typedef struct {

    void **elements;

    // number of elements
    int count;
    // the number of elements that can be held before resizing required
    int capacity;

    // returns whether the first argument
    // should go above hte second in the heap
    element_cmp want_first_above;

} heap;

heap *heap_new_empty(element_cmp want_first_above);
void heap_delete_only(heap *);
void heap_delete_and_elements(heap *, void (*delete_element)(void *));

heap *heap_from_array(void **, int length, element_cmp cmp);

void *heap_pop_top(heap *);

void heap_insert(heap *, void *);

int heap_count(heap *);

#endif // HEAP_H
