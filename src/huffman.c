
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "huffman.h"
#include "heap.h"

const size_t symbol_bitsize = 8;
const size_t num_symbols = 1 << 8;

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

// returns true if every node has 0 or 2 children, 
// and all parent pointers (excluding the root's) are correct
bool is_valid_tree(const tree_node *t) {
    if (t->left != NULL && t->right != NULL) {
        return t->left->parent == t
            && t->right->parent == t
            && is_valid_tree(t->left)
            && is_valid_tree(t->right);
    }
    return t->left == NULL && t->right == NULL;
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

bool is_shorter(const void *a, const void *b) {
    tree_node *nodeA = (tree_node *)a;
    tree_node *nodeB = (tree_node *)b;
    return height(nodeA) < height(nodeB);
}

// builds a tree for a prefix code over the symbols with nonzero frequency
tree_node *build_tree(const long *symbol_frequencies, bool (*merge_heuristic)(const void *, const void *)) {

    int num_present_symbols = 0;
    for (int i = 0; i < num_symbols; i++) {
        if (symbol_frequencies[i] > 0) {
            num_present_symbols++;
        }
    }
    if (num_present_symbols == 0) return NULL;

    bool add_dummy = false;
    int dummy = 0;
    if (num_present_symbols == 1) {
        // 1 symbol can fit in a tree of height 0
        // but a zero length code doesn't work
        // so include a dummy symbol in the code
        add_dummy = true;
        num_present_symbols++;
        
        // dummy must be different from the one real character
        const int def = 1, alt = 2;
        dummy = symbol_frequencies[def] == 0 ? def : alt;
    }

    tree_node **trees = malloc(sizeof(tree_node) * num_present_symbols);

    for (int i = 0, j = 0; i < num_symbols; i++) {
        if (symbol_frequencies[i] > 0 || (add_dummy && i == dummy)) {
            trees[j] = malloc(sizeof(tree_node));
            *trees[j] = (tree_node) {
                .parent = NULL,
                .left = NULL,
                .right = NULL,
                .total_frequency = symbol_frequencies[i],
                .symbol = (symbol) i
            };
            j++;
        }
    }

    heap *h = heap_from_array((void **)trees, num_present_symbols, merge_heuristic);

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

// build a prefix code where more common symbols have shorter codes
tree_node *build_huffman_tree(const long *symbol_frequencies) {
    return build_tree(symbol_frequencies, has_lower_frequency);
}

// build a prefix code where (present) symbols have (almost) uniform code length
tree_node *build_uniform_tree(const long *symbol_frequencies) {
    return build_tree(symbol_frequencies, is_shorter);
}

void delete_codes(bitstring **codes) {
    for (int i = 0; i < num_symbols; i++) {
        bitstring_delete(codes[i]);
    }
    free(codes);
}

bitstring **get_codes_from_tree(const tree_node *tree) {

    // NULL for symbols without a code
    bitstring **symbol_codes = calloc(num_symbols, sizeof(bitstring *));

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

tree_node *get_tree_from_codes(const bitstring **symbol_codes) {

    tree_node *root = malloc(sizeof(tree_node));
    *root = (tree_node) {
        .parent = NULL,
        .left = NULL,
        .right = NULL,
        .symbol = 0,
    };

    for (int i = 0; i < num_symbols; i++) {
        const bitstring *code = symbol_codes[i];
        if (code == NULL) continue; // don't insert symbols with no code

        tree_node *current = root;

        for (int k = 0; k < bitstring_bitlength(code); k++) {
            
            tree_node **childp;
            if (bitstring_get(code, k)) { // 1 = right
                childp = &(current->right);  // TODO: is this correct
            }else { // 0 = left
                childp = &(current->left);
            }

            if (*childp == NULL) {
                *childp = malloc(sizeof(tree_node));
                **childp = (tree_node) {
                    .parent = current,
                    .left = NULL,
                    .right = NULL,
                    .symbol = 0
                };
            }

            current = *childp;
        }

        // end of code path: symbol lives here
        current->symbol = (symbol) i; // is this cast ok?
    }

    return root;
}


bitstring *encode(const symbol *message, int message_length, const bitstring **symbol_codes) {
    bitstring *encoded = bitstring_new_empty();

    for (int i = 0; i < message_length; i++) {
        symbol s = message[i];

        const bitstring *codeword = symbol_codes[s]; // TODO: indexing with symbol bad ?

        bitstring_concat(encoded, codeword);
    }

    return encoded;
}

symbol *decode(const bitstring *encoded, const tree_node *tree, int *result_lengthp) {
    int capacity = 16;
    symbol *result = malloc(sizeof(symbol) * capacity);
    *result_lengthp = 0;

    int i = 0;
    while (i < bitstring_bitlength(encoded)) {
        const tree_node *current = tree;
        while (!is_leaf(current)) {
            
            if (i >= bitstring_bitlength(encoded)) {
                // incomplete final symbol   // TODO: or return partial?
                *result_lengthp = -1;
                free(result);
                return NULL;
            }

            if (bitstring_get(encoded, i++)) {
                current = current->right;
            }else {
                current = current->left;
            }
        }

        if (*result_lengthp == capacity) {
            capacity *= 2;
            result = realloc(result, sizeof(symbol) * capacity);
        }

        result[(*result_lengthp)++] = current->symbol;
    }
    return result;
}
