
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "bitstring.h"
#include "heap.h"

typedef unsigned char symbol;
const size_t symbol_bitsize = 8;
const size_t num_symbols = 1 << 8;

// a tree representing a prefix code
// either has 2 children, or 
// none and a symbol
// every node has a total frequency
typedef struct tree_node {
    
    struct tree_node *parent;

    // rename to zero / one ?
    struct tree_node *left;
    struct tree_node *right;

    symbol symbol;

    long total_frequency;

} tree_node;

bool is_leaf(const tree_node *t) {
    return t->left == NULL && t->right == NULL;
}

// free all nodes in a tree
void tree_delete(tree_node *t) {
    if (t->left != NULL) {
        tree_delete(t->left);
    }
    if (t->right != NULL) {
        tree_delete(t->right);
    }
    free(t);
}

int height(const tree_node *t) {
    if (is_leaf(t)) {
        return 0;
    }
    int hl = height(t->left) + 1;
    int hr = height(t->right) + 1;
    return (hl > hr) ? hl : hr;
}

bool has_lower_frequency(const void *a, const void *b) {
    tree_node *nodeA = (tree_node *)a;
    tree_node *nodeB = (tree_node *)b;

    return nodeA->total_frequency < nodeB->total_frequency;
}

bool is_shorter(void *a, void *b) {
    tree_node *nodeA = (tree_node *)a;
    tree_node *nodeB = (tree_node *)b;
    return height(nodeA) < height(nodeB);
}

tree_node *build_tree(const long *symbol_frequencies, bool (*merge_heuristic)(const void *, const void *)) {

    tree_node **trees = malloc(sizeof(tree_node) * num_symbols);

    for (int i = 0; i < num_symbols; i++) {
        trees[i] = malloc(sizeof(tree_node));
        *trees[i] = (tree_node) {
            .parent = NULL,
            .left = NULL,
            .right = NULL,
            .total_frequency = symbol_frequencies[i],
            .symbol = (symbol) i
        };
    }

    heap *h = heap_from_array((void **)trees, num_symbols, merge_heuristic);

    while (h->count > 1) {
        tree_node *t1 = heap_pop_top(h);
        tree_node *t2 = heap_pop_top(h);

        tree_node *combined = malloc(sizeof(tree_node));
        *combined = (tree_node) {
            .parent = NULL,
            .left = t1,     // TODO: which order ?
            .right = t2,
            .total_frequency = t1->total_frequency + t2->total_frequency,
            .symbol = (symbol) 0  // nothing (not a leaf)
        };

        t1->parent = combined;
        t2->parent = combined;
        
        heap_insert(h, combined);
    }

    tree_node *code_tree = heap_pop_top(h);

    heap_delete_only(h);
    free(trees);

    return code_tree;
}

bitstring **get_codes_from_tree(const tree_node *tree) {

    bitstring **symbol_codes = malloc(sizeof(bitstring *) * num_symbols);

    const tree_node *current = tree;
    
    // tracks path from root to current
    // false=left, true=right
    bitstring *current_code = bitstring_new_empty();

    // TODO: keeping code & current in sync is error prone, find a better solution
    while (true) {
        // DOWN (to first in this subtree)
        while (current->left != NULL) {
            bitstring_append(current_code, false);
            current = current->left;
        }
        // now at a symbol
        symbol_codes[current->symbol] = bitstring_copy(current_code);

        // UP (get out of this subtree)
        while (true) {
            current = current->parent;
            bool dir = bitstring_pop(current_code);
            if (!dir) { // popped a zero -> exited a left subtree
                break;
            }
            if (current->parent == NULL) { // read root from right subtree -> done
                bitstring_delete(current_code);
                return symbol_codes;
            }
        }

        // finished exploring left subtree of current,
        // now explore right subtree
        current = current->right;
        bitstring_append(current_code, true);

    }
}


bitstring *encode(const symbol *message, int message_length, bitstring **symbol_codes) {
    bitstring *encoded = bitstring_new_empty();

    for (int i = 0; i < message_length; i++) {
        symbol s = message[i];

        const bitstring *codeword = symbol_codes[s]; // TODO: indexing with symbol bad ?

        bitstring_concat(encoded, codeword);
    }

    return encoded;
}

int main(int argc, char const *argv[]) {
    
    const char *message = "the quick brown fox jumps over the lazy dog";

    long *frequencies = calloc(num_symbols, sizeof(long));
    for (int i = 0; message[i] != '\0'; i++) {
        frequencies[(symbol)message[i]]++;
    }

    tree_node *code_tree = build_tree(frequencies, has_lower_frequency);
    free(frequencies);

    bitstring **codes = get_codes_from_tree(code_tree);
    tree_delete(code_tree);

    bitstring *encoded = encode((const symbol *)message, strlen(message), codes);
    printf("original %ld bytes, compressed %d bytes\n", 
        strlen(message), 
        (bitstring_bitlength(encoded) + 7) / 8);
    bitstring_delete(encoded);

    for (int i = 0; i < num_symbols; i++) {
        bitstring_delete(codes[i]);
    }
    free(codes);
    
    return 0;
}
