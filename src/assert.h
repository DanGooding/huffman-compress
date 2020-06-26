
#define assert(cond, message)\
    if (!(cond)) {\
        fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, (message));\
        exit(1);\
    }
