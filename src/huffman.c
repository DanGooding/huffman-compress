
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
    int dummy;
    if (num_present_symbols == 1) {
        // 1 symbol can fit in a tree of height 0
        // but a zero length code doesn't work
        // so include a dummy symbol in the code
        add_dummy = true;
        num_present_symbols++;
        
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

void compress(const char *src_filename, const char *dest_filename) {

    FILE *f_src = fopen(src_filename, "rb");
    if (f_src == NULL) {
        fprintf(stderr, "failed to open %s\n", src_filename);
        exit(1);
    }
    
    fseek(f_src, 0, SEEK_END);
    long src_size = ftell(f_src);
    rewind(f_src);

    unsigned char *content = malloc(sizeof(unsigned char) * src_size);
    
    if (fread(content, sizeof(unsigned char), src_size, f_src) != src_size) {
        fprintf(stderr, "failed to read whole of %s\n", src_filename);
        fclose(f_src);
        free(content);
        exit(1);
    }

    fclose(f_src);


    long *symbol_frequencies = calloc(num_symbols, sizeof(long));
    for (int i = 0; i < src_size; i++) {
        symbol_frequencies[content[i]]++;
    }

    tree_node *tree = build_huffman_tree(symbol_frequencies);
    free(symbol_frequencies);

    bitstring **codes = get_codes_from_tree(tree);
    tree_delete(tree);


    bitstring *encoded = encode(content, src_size, (const bitstring **)codes);
    free(content);
    

    // TODO: don't overwrite an existing file -- race condition
    FILE *f_dest = fopen(dest_filename, "wb");
    if (f_dest == NULL) {
        fprintf(stderr, "failed to open %s for writing\n", dest_filename);
        
        delete_codes(codes);
        bitstring_delete(encoded);

        exit(1);
    }

    // write the symbols' codes
    bitstring *empty_bitstring = bitstring_new_empty();
    for (int i = 0; i < num_symbols; i++) {
        
        const bitstring *code = (codes[i] == NULL) ? empty_bitstring : codes[i];
        bool success = bitstring_write(code, f_dest);

        if (!success) {
            fprintf(stderr, "error saving codes\n");

            delete_codes(codes);
            bitstring_delete(encoded);
            fclose(f_dest);
            bitstring_delete(empty_bitstring);

            exit(1);
        }
    }
    delete_codes(codes);
    bitstring_delete(empty_bitstring);

    // write the encoded content
    if (!bitstring_write(encoded, f_dest)) {
        fprintf(stderr, "error saving encoded string\n");

        bitstring_delete(encoded);
        fclose(f_dest);
        exit(1);
    }

    bitstring_delete(encoded);
    fclose(f_dest);
}

void decompress(const char *src_filename, const char *dest_filename) {

    FILE *f_src = fopen(src_filename, "rb");
    if (f_src == NULL) {
        fprintf(stderr, "failed to open %s\n", src_filename);
        exit(1);
    }

    bitstring **codes = malloc(sizeof(bitstring *) * num_symbols);
    for (int i = 0; i < num_symbols; i++) {
        bitstring *code = bitstring_read(f_src);

        if (code == NULL) {
            fprintf(stderr, "error reading codes from %s\n", src_filename);
            fclose(f_src);
            delete_codes(codes);
            exit(1);

        }else if (bitstring_bitlength(code) == 0) {
            // a zero bitstring is saved to indicate this symbol has no code
            codes[i] = NULL;
            bitstring_delete(code);
        }else {
            codes[i] = code;
        }
    }

    bitstring *encoded = bitstring_read(f_src);
    if (encoded == NULL) {
        fprintf(stderr, "error reading encoded content\n");
        fclose(f_src);
        delete_codes(codes);
        exit(1);
    }

    // TODO: assert: at end of file
    fclose(f_src);

    tree_node *cmp_tree = get_tree_from_codes((const bitstring **)codes);
    delete_codes(codes);

    int decoded_size;
    symbol *decoded = decode(encoded, cmp_tree, &decoded_size);
    bitstring_delete(encoded);
    tree_delete(cmp_tree);


    FILE *f_dest = fopen(dest_filename, "wb");
    if (f_dest == NULL) {
        fprintf(stderr, "failed to open %s for writing\n", dest_filename);
        free(decoded);
        exit(1);
    }
    if (fwrite(decoded, sizeof(symbol), decoded_size, f_dest) != decoded_size) {
        fprintf(stderr, "failed to write to %s\n", dest_filename);
        free(decoded);
        fclose(f_dest);
        exit(1);
    }
    fclose(f_dest);
    free(decoded);
}

int main(int argc, char const *argv[]) {
    
    const char *original = "alice.txt";
    const char *compressed = "alice.txt.hffmn";
    const char *decompressed = "alice1.txt";

    compress(original, compressed);
    decompress(compressed, decompressed);

    return 0;
}
