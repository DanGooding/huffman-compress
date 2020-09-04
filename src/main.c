
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitstring.h"
#include "huffman.h"

void compress(const char *src_filename, const char *dest_filename) {

    FILE *f_src = fopen(src_filename, "rb");
    if (f_src == NULL) {
        fprintf(stderr, "failed to open %s\n", src_filename);
        exit(1);
    }
    
    long *symbol_frequencies = calloc(num_symbols, sizeof(long));

    int capacity = 1 << 15;  // TODO: find a good size
    unsigned char *buf = malloc(sizeof(unsigned char) * capacity);
    int nread;
    while ((nread = fread(buf, sizeof(unsigned char), capacity, f_src)) > 0) {
        for (int i = 0; i < nread; i++) {
            symbol_frequencies[buf[i]]++;
        }
    }

    tree_node *tree = build_huffman_tree(symbol_frequencies);
    free(symbol_frequencies);

    bitstring **codes = get_codes_from_tree(tree);
    tree_delete(tree);

    // TODO: don't overwrite an existing file -- (avoid race condition when fix)
    FILE *f_dest = fopen(dest_filename, "wb");
    if (f_dest == NULL) {
        fprintf(stderr, "failed to open %s for writing\n", dest_filename);
        
        fclose(f_src);
        delete_codes(codes);

        exit(1);
    }

    // write the symbols' codes
    bitstring *empty_bitstring = bitstring_new_empty();
    for (int i = 0; i < num_symbols; i++) {
        
        const bitstring *code = (codes[i] == NULL) ? empty_bitstring : codes[i];
        bool success = bitstring_write(code, f_dest);

        if (!success) {
            fprintf(stderr, "error saving codes\n");

            fclose(f_src);
            delete_codes(codes);
            fclose(f_dest);
            bitstring_delete(empty_bitstring);

            exit(1);
        }
    }
    bitstring_delete(empty_bitstring);

    
    rewind(f_src);
    while ((nread = fread(buf, sizeof(unsigned char), capacity, f_src)) > 0) {
        bitstring *encoded = encode(buf, nread, (const bitstring **)codes);
        bool success = bitstring_write(encoded, f_dest);
        bitstring_delete(encoded);
        if (!success) {
            fprintf(stderr, "error saving content\n");

            fclose(f_src);
            delete_codes(codes);
            fclose(f_dest);
            free(buf);

            exit(1);
        }
    }
    fclose(f_src);
    delete_codes(codes);
    fclose(f_dest);
    free(buf);
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

    tree_node *tree = get_tree_from_codes((const bitstring **)codes);
    delete_codes(codes);

    FILE *f_dest = fopen(dest_filename, "wb");

    bitstring *encoded;
    while ((encoded = bitstring_read(f_src)) != NULL) {
        int decoded_length;
        unsigned char *decoded = decode(encoded, tree, &decoded_length);
        bitstring_delete(encoded);
        if (fwrite(decoded, sizeof(unsigned char), decoded_length, f_dest) != decoded_length) {
            fprintf(stderr, "error writing to file\n");
            fclose(f_src);
            fclose(f_dest);
            tree_delete(tree);
            free(decoded);
            exit(1);
        }
        free(decoded);
    }

    fclose(f_src);
    fclose(f_dest);
    tree_delete(tree);
}

int main(int argc, char const *argv[]) {
    
    bool mode_compress = true;
    int i = 1;
    if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compress") == 0) {
        mode_compress = true;
        i++;
    }else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--decompress") == 0) {
        mode_compress = false;
        i++;
    }

    const char *src_filename = argv[i++];
    const char *dest_filename = argv[i++];

    if (mode_compress) {
        compress(src_filename, dest_filename);
    }else {
        decompress(src_filename, dest_filename);
    }

    return 0;
}

