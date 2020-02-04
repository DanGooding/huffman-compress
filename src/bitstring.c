
#include <stdbool.h>
#include <stdlib.h>

#include "bitstring.h"

const size_t INITIAL_CAPACITY_BYTES = 1;

// TODO: remove magic numbers, functions for eg round up to byte etc

bitstring *bitstring_new_with_capacity(size_t byte_capacity) {
    bitstring *bits = malloc(sizeof(bitstring));
    if (bits == NULL) return NULL;

    bits->byte_capacity = byte_capacity;
    bits->length = 0;

    bits->bytes = malloc(sizeof(char) * bits->byte_capacity);

    return bits;
}

bitstring *bitstring_new_empty() {
    return bitstring_new_with_capacity(INITIAL_CAPACITY_BYTES);
}
void bitstring_delete(bitstring *bits) {
    free(bits->bytes);
    free(bits);
}

bitstring *bitstring_copy(const bitstring *original) {
    // TODO: power of two size?
    bitstring *bits = bitstring_new_with_capacity((original->length + 7) / 8);

    for (int i = 0; i < original->length; i++) {
        bits->length++;
        bitstring_set(bits, i, bitstring_get(original, i));
    }

    return bits;
}

bool bitstring_get(const bitstring *bits, int i) {
    if (i < 0 || i >= bits->length) {
        return false;
    }
    char byte = bits->bytes[i / 8];
    // bit 0 is highest, bit 7 is lowest
    int j = 7 - i % 8;
    return (byte >> j) & 1;
}

void bitstring_set(bitstring *bits, int i, bool b) {
    if (i < 0 || i >= bits->length) {
        return;
    }
    // bit 0 is highest, 7 is lowest
    char selector = 1 << (7 - i % 8);
    if (b) {
        bits->bytes[i / 8] |= selector;
    }else {
        bits->bytes[i / 8] &= ~selector;
    }
}

void bitstring_append(bitstring *bits, bool b) {
    if (bits->length == bits->byte_capacity * 8) {
        bits->byte_capacity *= 2;
        bits->bytes = realloc(bits->bytes, sizeof(char) * bits->byte_capacity);
    }
    bits->length++;
    bitstring_set(bits, bits->length - 1, b);
}

void bitstring_concat(bitstring *bits, const bitstring *other_bits) {
    if (other_bits->length == 0) {
        return;
    }
    size_t new_length = bits->length + other_bits->length;
    if (new_length > bits->byte_capacity * 8) {

        // // increase capacity so sufficient to hold other_bits
        // bits->byte_capacity = (new_length + 7) / 8;
        
        // double until sufficient to hold other_bits
        do {
            bits->byte_capacity *= 2;
        } while (new_length > bits->byte_capacity * 8);
        
        bits->bytes = realloc(bits->bytes, bits->byte_capacity);
    }

    for (int i = 0; i < other_bits->length; i++) {
        // TODO: copy byte by byte ? using bit shifting
        bitstring_append(bits, bitstring_get(other_bits, i));
    }

}

bitstring *bitstring_substring(const bitstring *bits, int start, int stop) {
    if (start < 0) start = 0;
    if (stop >= bits->length) stop = bits->length;

    int length = stop - start;
    if (length <= 0) {
        return bitstring_new_empty();
    }

    bitstring *sub_bits = bitstring_new_with_capacity((length + 7) / 8);
    sub_bits->length = length;

    for (int i = 0; i < length; i++) {
        bitstring_set(sub_bits, i, bitstring_get(bits, start + i));
    }

    return sub_bits;
}

const char *bitstring_to_bytes(const bitstring *bits) {
    // TODO: give number also ? - else useless
    return bits->bytes;
}