#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "bitstring.h"

typedef unsigned char symbol;
extern const size_t symbol_bitsize;
extern const size_t num_symbols;

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


tree_node *build_huffman_tree(const long *symbol_frequencies);
void tree_delete(tree_node *t);

bitstring **get_codes_from_tree(const tree_node *tree);
tree_node *get_tree_from_codes(const bitstring **symbol_codes);
void delete_codes(bitstring **codes);

bitstring *encode(const symbol *message, int message_length, const bitstring **symbol_codes);
symbol *decode(const bitstring *encoded, const tree_node *tree, int *result_lengthp);

#endif // HUFFMAN_H
