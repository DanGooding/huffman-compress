
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "heap.h"

const int INITIAL_CAPACITY = 16;

heap *heap_new_with_capacity(element_cmp want_first_above, int capacity) {
    heap *h = malloc(sizeof(heap));
    if (h == NULL) {
        return NULL;
    }

    h->count = 0;
    h->capacity = capacity;
    h->want_first_above = want_first_above;
    
    h->elements = malloc(sizeof(void *) * h->capacity);

    return h;
}

heap *heap_new_empty(element_cmp cmp) {
    return heap_new_with_capacity(cmp, INITIAL_CAPACITY);
}

void heap_delete_only(heap *h) {
    free(h->elements);
    free(h);
}

void heap_delete_and_elements(heap *h, void (*delete_element)(void *)) {
    for (int i = 0; i < h->count; i++) {
        delete_element(h->elements[i]);
    }
    heap_delete_only(h);
}

// swap element at index i downward to restore heap property
void swap_down(heap *h, int i) {
    
    if (i < 0 || i >= h->count) return;

    int left_i;
    while ((left_i = 2 * i + 1) < h->count) { 
        // while has a leaf

        int right_i = left_i + 1;

        int priority_i;

        if (right_i < h->count) { 
            // both children
           
            if (h->want_first_above(
                    h->elements[left_i], 
                    h->elements[right_i])) {
                priority_i = left_i;
            }else {
                priority_i = right_i;
            }

        }else { 
            // only left child
            priority_i = left_i;
        }

        
        if (h->want_first_above(
                h->elements[priority_i],
                h->elements[i])) { 
            // max child larger than parent
            
            void *tmp = h->elements[i];
            h->elements[i] = h->elements[priority_i];
            h->elements[priority_i] = tmp;
            i = priority_i;

        }else {
            // parent larger than both children
            // heap property holds

            return;
        }

    }
}

// from array of elements or pointers to elements ?
heap *heap_from_array(void **elements, int length, element_cmp cmp) {
    
    heap *h = heap_new_with_capacity(cmp, length);

    for (int i = 0; i < length; i++) {
        h->elements[i] = elements[i]; // shallow copy
    }
    h->count = length;

    // heapify
    for (int i = h->count - 1; i >= 0; i--) {
        swap_down(h, i);
    }

    return h;
}

void *heap_pop_top(heap *h) {

    if (h->count == 0) {
        return NULL;
    }

    // save top
    void *top = h->elements[0];

    // replace with bottom (decreasing count)
    h->elements[0] = h->elements[--h->count];

    // swap down to restore heap property
    swap_down(h, 0);

    return top;
}

void heap_insert(heap *h, void *e) {

    if (h->count == h->capacity) {
        
        h->capacity *= 2;
        h->elements = realloc(h->elements, sizeof(void *) * h->capacity);
    }

    h->elements[h->count++] = e;

    // swap up to restore heap property
    int i = h->count - 1;
    while (i > 0) {
        int parent_i = (i - 1) / 2;

        if (h->want_first_above(
                h->elements[i],
                h->elements[parent_i])) {
            
            void *tmp = h->elements[i];
            h->elements[i] = h->elements[parent_i];
            h->elements[parent_i] = tmp;

            i = parent_i;

        }else {
            return;
        }

    }


}








