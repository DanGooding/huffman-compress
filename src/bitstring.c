
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "bitstring.h"
#include "writeutils.h"

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
    if (bits == NULL) return;
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

bool bitstring_pop(bitstring *bits) {
    if (bits->length == 0) {
        return false;
    }
    bool last = bitstring_get(bits, bits->length - 1);
    bits->length--;
    return last;
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

    
    int offset = bits->length % 8;

    int num_bytes_to_copy = (other_bits->length + 7) / 8;
    int start_byte = bits->length / 8;

    // copy bytes from other_bits (src) to bits (dest), 
    // shifting along if bits doesn't end on a byte boundary
    for (int i = 0; i < num_bytes_to_copy; i++) {
        if (i == 0) {
            // zero only the last `8 - offset` bits
            bits->bytes[start_byte + i] &= ~ ((1 << (8 - offset)) - 1);
        }
        // copy high bits from source into to low bits of dest
        bits->bytes[start_byte + i] |= ((unsigned char)other_bits->bytes[i]) >> offset;

        if (i < num_bytes_to_copy - 1 || 
            other_bits->length - (num_bytes_to_copy - 1) * 8 > 8 - offset) {
            // zero the next byte and copy the low bits of this byte into it 
            //  may not be done for the last byte of src if its occupied bits (LHS of inequality)
            //  fit into the previously filled byte of dest (RHS)

            // zero whole of next byte (current zeroed last time)
            bits->bytes[start_byte + i + 1] = 0;

            // copy low bits from source into high bits of next byte of dest
            bits->bytes[start_byte + i + 1] |= other_bits->bytes[i] << (8 - offset);
        }
    }
    bits->length = new_length;
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

char *bitstring_show(const bitstring *bits) {
    char *str = malloc(sizeof(char) * (bits->length + 1));
    str[bits->length] = '\0';
    for (int i = 0; i < bits->length; i++) {
        str[i] = bitstring_get(bits, i) ? '1' : '0';
    }
    return str;
}

int bitstring_bitlength(const bitstring *bits) {
    return bits->length;
}

bool bitstring_equals(const bitstring *bits1, const bitstring *bits2) {
    if (bits1->length != bits2->length) {
        return false;
    }
    // TODO: speedup by comparing full bytes
    for (int i = 0; i < bits1->length; i++) {
        if (bitstring_get(bits1, i) != bitstring_get(bits2, i)) {
            return false;
        }
    }
    return true;
}

bool bitstring_write(const bitstring *bits, FILE *f) {
    int bitlength = bits->length;

    if (!write_int(bitlength, f)) {
        return false;
    }
    if (bitlength == 0) {
        return true;
    }

    int byte_length = (bitlength + 7) / 8; // round up

    // zero out the bits which pad this to a whole number of bytes
    // (so we totally control what is written)
    int padding = byte_length * 8 - bitlength;
    bits->bytes[byte_length - 1] &= ~((1 << padding) - 1);
    
    if (fwrite(bits->bytes, sizeof(char), byte_length, f) != byte_length) {
        return false;
    }
    return true;
}

bitstring *bitstring_read(FILE *f) {
    int bitlength = -1;

    if (!read_int(&bitlength, f)) {
        return NULL;
    }
    
    if (bitlength < 0) {
        return NULL;
    }else if (bitlength == 0) {
        return bitstring_new_empty();
    }

    bitstring *bits = bitstring_new_with_capacity(bitlength);

    int bytelength = (bitlength + 7) / 8;
    if (fread(bits->bytes, sizeof(char), bytelength, f) != bytelength) {
        bitstring_delete(bits);
        return NULL;
    }
    bits->length = bitlength;

    return bits;
}
