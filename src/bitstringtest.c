
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "bitstring.h"
#include "assert.h"

#define BIN_FORMAT "%d%d%d%d%d%d%d%d"
#define BIN_PRINTF(n) \
    n & 0x80 ? 1 : 0,\
    n & 0x40 ? 1 : 0,\
    n & 0x20 ? 1 : 0,\
    n & 0x10 ? 1 : 0,\
    n & 0x08 ? 1 : 0,\
    n & 0x04 ? 1 : 0,\
    n & 0x02 ? 1 : 0,\
    n & 0x01 ? 1 : 0

bitstring *make_random_bitstring(int length) {
    bitstring *bits = bitstring_new_empty();
    for (int i = 0; i < length; i++) {
        bitstring_append(bits, (rand() % 2) == 0);
    }
    return bits;
}

bool is_prime(int n) {
    if (n < 2) return false;
    for (int i = 2; i < n; i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

const int n = 10000;

int main() {

    bitstring *primalities = bitstring_new_empty();

    assert(primalities->length == 0, "length of empty should be zero");

    for (int i = 0; i < n; i++) {
        bitstring_append(primalities, is_prime(i));
    }

    assert(primalities->length == n, "length should be equal to number of items added");

    for (int i = 0; i < n; i++) {
        assert(bitstring_get(primalities, i) == is_prime(i), "get should return appended value");
    }

    bitstring_set(primalities, 1, true);
    assert(bitstring_get(primalities, 1), "set(true) should change value");
    bitstring_set(primalities, 1, false);
    assert(!bitstring_get(primalities, 1), "set(false) should change value");

    const int k = 20;
    for (int i = 0; i < k; i++) {
        bitstring_append(primalities, i % 3 == 0);
    }
    int prev_length = primalities->length;
    for (int i = k-1; i >= 0; i--) {
        bool b = bitstring_pop(primalities);
        assert(b == (i % 3 == 0), "pop should return correct value");
    }
    assert(primalities->length == prev_length - k, "pop should decrease length by number popped");

    bitstring *double_digits = bitstring_substring(primalities, 10, 100);

    assert(double_digits->length == 90, "substring length should be length of range");



    for (int i = 0; i < double_digits->length; i++) {
        assert(
            bitstring_get(double_digits, i) == bitstring_get(primalities, i + 10), 
            "substring values should be same as superstring");
    }

    bitstring *multiple_digits = bitstring_copy(double_digits);
    assert(multiple_digits->length == double_digits->length, "copy should have same length");
    for (int i = 0; i < double_digits->length; i++) {
        assert(
            bitstring_get(multiple_digits, i) == bitstring_get(double_digits, i), 
            "copy should have same values");
    }

    bitstring *triple_plus_digits = bitstring_substring(primalities, 100, primalities->length);
    assert(triple_plus_digits->length == primalities->length - 100, "substring should give correct length");
    for (int i = 0; i < triple_plus_digits->length; i++) {
        assert(bitstring_get(triple_plus_digits, i) == bitstring_get(primalities, i + 100),
            "substring should copy values");
    }

    bitstring_concat(multiple_digits, triple_plus_digits);
    bitstring_delete(triple_plus_digits);
    assert(multiple_digits->length == n - 10, "concat should sum lengths");

    for (int i = 0; i < multiple_digits->length; i++) {
        assert(
            bitstring_get(primalities, i + 10) == bitstring_get(multiple_digits, i),
            "concat should preserve values"
        );
    }


    const char low_prime_bytes[] = {
        0x35, // 0b00110101, // 0 - 7
        0x14, // 0b00010100, // 8 - 15
        0x51  // 0b01010001  // 16 - 23
    };

    const char *bytes = bitstring_to_bytes(primalities);

    for (int i = 0; i < sizeof(low_prime_bytes); i++) {
        assert(low_prime_bytes[i] == bytes[i], "to_bytes should give correct bytes");
    }

    char *str = bitstring_show(primalities);
    assert(strncmp(str, "001101010001010001010001", 24) == 0, "show should give correct string");
    free(str);

    bitstring_delete(primalities);
    bitstring_delete(double_digits);
    bitstring_delete(multiple_digits);


    srand(42);
    bitstring **strings = malloc(sizeof(bitstring *) * n);
    bitstring *all = bitstring_new_empty();

    for (int i = 0; i < n; i++) {
        strings[i] = make_random_bitstring(i % 30);
        bitstring_concat(all, strings[i]);
    }

    int base = 0;
    for (int i = 0; i < n; i++) {
        bitstring *sub = bitstring_substring(all, base, base + bitstring_bitlength(strings[i]));
        assert(bitstring_equals(sub, strings[i]), "concat should preserve substring");
        base += bitstring_bitlength(strings[i]);
        bitstring_delete(sub);
        bitstring_delete(strings[i]);
    }
    bitstring_delete(all);
    free(strings);
    
    return 0;
}





