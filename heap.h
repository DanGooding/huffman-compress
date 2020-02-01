
#include <stdbool.h>

typedef bool (*element_cmp) (int, int);

// TODO hide internals by making heap* alias of void*
typedef struct {

    int *elements;

    // number of elements
    int count;
    // the number of elements that can be held before resizing required
    int capacity;

    // returns whether the first argument
    // should go above hte second in the heap
    element_cmp want_first_above;

} heap;

heap *heap_new_empty(element_cmp want_first_above);
void heap_delete(heap *);

heap *heap_from_array(int *, int);

int heap_pop_top(heap *);

void heap_insert(heap *, int);



