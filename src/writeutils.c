
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "writeutils.h"

// write the 4 bytes of an int as big endian
// most significant byte at lowest address
// returns true on success
bool write_uint(uint32_t i, FILE *f) {  // TODO: is the conversion to unsigned ok?
    uint8_t buf[4];
    buf[0] = (i >> 24) & 0xff;
    buf[1] = (i >> 16) & 0xff;
    buf[2] = (i >>  8) & 0xff;
    buf[3] = (i      ) & 0xff;
    return fwrite(buf, sizeof(uint8_t), 4, f) == 4;
}

bool write_int(int32_t i, FILE *f) {
    return write_uint((uint32_t)i, f);
}

// read the 4 bytes of an int as big endian
// returns true on success
bool read_uint(uint32_t *i, FILE *f) {
    uint8_t buf[4];
    if (fread(buf, sizeof(uint8_t), 4, f) != 4) {
        return false;
    }
    *i = 0;
    *i |= (uint32_t)buf[0] << 24;
    *i |= (uint32_t)buf[1] << 16;
    *i |= (uint32_t)buf[2] << 8;
    *i |= (uint32_t)buf[3];
    return true;
}

bool read_int(int32_t *i, FILE *f) {
    return read_uint((uint32_t *)i, f);
}

// write the 8 bytes of a long as bit endian
// returns true on success
bool write_ulong(uint64_t i, FILE *f) {  // TODO: is the conversion to unsigned ok?
    uint8_t buf[8];
    buf[0] = (i >> 56) & 0xff;
    buf[1] = (i >> 48) & 0xff;
    buf[2] = (i >> 40) & 0xff;
    buf[3] = (i >> 32) & 0xff;
    buf[4] = (i >> 24) & 0xff;
    buf[5] = (i >> 16) & 0xff;
    buf[6] = (i >>  8) & 0xff;
    buf[7] = (i      ) & 0xff;
    return fwrite(buf, sizeof(uint8_t), 8, f) == 8;
}

bool write_long(int64_t i, FILE *f) {
    return write_ulong((uint64_t)i, f);
}

// read the 8 bytes of a long as big endian
// returns true on success
bool read_ulong(uint64_t *i, FILE *f) {
    uint8_t buf[8];
    if (fread(buf, sizeof(uint8_t), 8, f) != 8) {
        return false;
    }
    *i = 0;
    *i |= (uint64_t)buf[0] << 56;
    *i |= (uint64_t)buf[1] << 48;
    *i |= (uint64_t)buf[2] << 40;
    *i |= (uint64_t)buf[3] << 32;
    *i |= (uint64_t)buf[4] << 24;
    *i |= (uint64_t)buf[5] << 16;
    *i |= (uint64_t)buf[6] << 8;
    *i |= (uint64_t)buf[7];
    return true;
}

bool read_long(int64_t *i, FILE *f) {
    return read_ulong((uint64_t *)i, f);
}
