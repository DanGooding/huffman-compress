
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "huffman.h"

typedef tree_node *(*tree_builder)(const long *);

void assert_tree_valid(tree_node *t) {
    // assert(t->total_frequency >= 0, "frequency cannot be negative");

    if (t->left != NULL && t->right != NULL) {

        assert(t->left->parent == t, "left child's parent must be self");
        assert(t->right->parent == t, "right child's parent must be self");

        assert_tree_valid(t->left);
        assert_tree_valid(t->right);

    }else {
        assert(t->left == NULL && t->right == NULL, "node must have 2 or 0 children");
    }
}

bool trees_equal(const tree_node *t1, const tree_node *t2) {
    if (t1 != NULL && t2 != NULL) {
        if (t1->left == NULL && t1->right == NULL 
         && t2->left == NULL && t2->right == NULL) {
            return t1->symbol == t2->symbol;
        }
        return trees_equal(t1->left, t2->left)
            && trees_equal(t1->right, t2->right);
    }
    return t1 == NULL && t2 == NULL;
}

long *count_symbols(const symbol *message, int message_length) {
    long *symbol_frequencies = calloc(num_symbols, sizeof(long));
    for (int i = 0; i < message_length; i++) {
        symbol_frequencies[message[i]]++;
    }
    return symbol_frequencies;
}

void test_for_input(tree_builder build_tree, const symbol *message, int message_length) {

    long *symbol_frequencies = count_symbols(message, message_length);

    tree_node *tree = build_tree(symbol_frequencies);
    assert_tree_valid(tree);

    bitstring **symbol_codes = get_codes_from_tree(tree);
    for (int i = 0; i < num_symbols; i++) {
        if (symbol_frequencies[i] > 0) {
            assert(symbol_codes[i] != NULL, "present symbols should be assigned a code");
        }
    }
    free(symbol_frequencies);

    tree_node *tree_again = get_tree_from_codes((const bitstring **)symbol_codes);
    assert(trees_equal(tree, tree_again), "get_codes_from_tree and get_tree_from_codes should be inverses");

    bitstring *encoded = encode(message, message_length, (const bitstring **)symbol_codes);
    int decoded_length;
    symbol *decoded = decode(encoded, tree_again, &decoded_length);
    assert(message_length == decoded_length, "encoding & decoding should preserve message length");
    assert(memcmp(message, decoded, decoded_length) == 0, "encode and decode should be inverses");

    bitstring_delete(encoded);
    free(decoded);

    delete_codes(symbol_codes);
    tree_delete(tree);
    tree_delete(tree_again);
}

int main() {
    
    tree_builder builders[] = {
        build_huffman_tree,
        build_uniform_tree
    };
    char *builder_names[] = {
        "huffman code",
        "uniform code"
    };
    for (int i = 0; i < 2; i++) {
        printf("%s\n", builder_names[i]);

        printf("test for english sentence\n");
        const char *sentence = "the quick brown fox jumps over the lazy dog.";
        test_for_input(builders[i], (symbol *)sentence, strlen(sentence));

        printf("test for single repeated character\n");
        const char *single = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
        test_for_input(builders[i], (symbol *)single, strlen(single));

        printf("test for few characters\n");
        const char *few = "abbbbbbccbcbbbbbabcbbbcbbbcabbabbcbabcbbbbbccbcbcbbcbabbabbcba";
        test_for_input(builders[i], (symbol *)few, strlen(few));
        
        printf("test for every symbol\n");
        symbol *all = malloc(sizeof(symbol) * num_symbols);
        for (int i = 0; i < num_symbols; i++) {
            all[i] = (symbol)i;
        }
        test_for_input(builders[i], all, num_symbols);
        free(all);
    }


    return 0;
}




