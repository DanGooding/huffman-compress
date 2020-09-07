#ifndef WRITEUTILS_H
#define WRITEUTILS_H

#include <stdint.h>
#include <stdio.h>

bool write_uint(uint32_t, FILE *);
bool write_int(int32_t, FILE *);
bool read_uint(uint32_t *, FILE *);
bool read_int(int32_t *, FILE *);

bool write_ulong(uint64_t, FILE *);
bool write_long(int64_t, FILE *);
bool read_ulong(uint64_t *, FILE *);
bool read_long(int64_t *, FILE *);

#endif // WRITEUTILS_H
