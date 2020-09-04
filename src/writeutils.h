#ifndef WRITEUTILS_H
#define WRITEUTILS_H

#include <stdint.h>
#include <stdio.h>

bool write_int(uint32_t, FILE *);
bool read_int(uint32_t *, FILE *);

bool write_long(uint64_t, FILE *);
bool read_long(uint64_t *, FILE *);

#endif // WRITEUTILS_H
