
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "writeutils.h"
#include "assert.h"

int main() {

    FILE *f = tmpfile();

    const int n = 1000;
    bool is_int[n];
    int ints[n];
    long longs[n];

    srand(42);
    for (int i = 0; i < n; i++) {
        if (rand() % 2) {
            is_int[i] = true;
            ints[i] = rand();
            bool success = write_int(ints[i], f);
            assert(success, "write int should succeed");
        }else {
            is_int[i] = false;
            longs[i] = random();
            bool success = write_long(longs[i], f);
            assert(success, "write long should succeed");
        }
    }

    rewind(f);
    for (int i = 0; i < n; i++) {
        if (is_int[i]) {
            int x;
            bool success = read_int(&x, f);
            assert(success, "read int should succeed");
            assert(x == ints[i], "read should recover same int");
        }else {
            long x;
            bool success = read_long(&x, f);
            assert(success, "read long should succeed");
            assert(x == longs[i], "read should recover same long");
        }
    }

    fclose(f);

    return 0;
}




