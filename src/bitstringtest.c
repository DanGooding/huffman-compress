
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "bitstring.h"

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

void assert(bool b, const char *message) {
    if (!b) {
        fprintf(stderr, "assertion failed: %s\n", message);
        abort();
    }
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

int main(int argc, char const *argv[]) {

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

    bitstring *multile_digits = bitstring_copy(double_digits);
    assert(multile_digits->length == double_digits->length, "copy should have same length");
    for (int i = 0; i < double_digits->length; i++) {
        assert(
            bitstring_get(multile_digits, i) == bitstring_get(double_digits, i), 
            "copy should have same values");
    }

    bitstring_concat(multile_digits, bitstring_substring(primalities, 100, primalities->length));
    assert(multile_digits->length == n - 10, "concat should sum lengths");

    for (int i = 0; i < multile_digits->length; i++) {
        assert(
            bitstring_get(primalities, i + 10) == bitstring_get(multile_digits, i),
            "concat should preserve values"
        );
    }


    const char low_prime_bytes[] = {
        0b00110101, // 0 - 7
        0b00010100, // 8 - 15
        0b01010001  // 16 - 23
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
    bitstring_delete(multile_digits);

    return 0;
}





