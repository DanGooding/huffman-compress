#ifndef ASSERT_H
#define ASSERT_H

#include <stdio.h>

#define assert(cond, message)\
    if (!(cond)) {\
        fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, (message));\
        assert_breakpoint();\
        exit(1);\
    }

void assert_breakpoint();

#endif // ASSERT_H
