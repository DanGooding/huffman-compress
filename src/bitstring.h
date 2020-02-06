
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    
    // array of each 8 bits
    char *bytes;  // TODO: store internally as longs instead ?
    
    // number of allocated bytes in `bytes` buffer
    size_t byte_capacity;
    
    // current length in bits
    size_t length;

} bitstring;

bitstring *bitstring_new_empty();
void bitstring_delete(bitstring *);
bitstring *bitstring_copy(const bitstring *);

bool bitstring_get(const bitstring *, int i);
void bitstring_set(bitstring *, int i, bool b);

void bitstring_append(bitstring *, bool b);
bool bitstring_pop(bitstring *);

void bitstring_concat(bitstring *bits, const bitstring *other_bits);

bitstring *bitstring_substring(const bitstring *, int start, int stop);

// TODO: is this a bit pointless ?
//  have a serialization + (deserialisation) function ?
const char *bitstring_to_bytes(const bitstring *);

// TODO: constructor from bool array ?
