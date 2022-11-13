#include "liboblivious/primitives.h"

void *o_memcpy(void *restrict dest, const void *restrict src, size_t n, bool cond) {
    for (size_t i = 0; i < n; i++) {
        o_set8((uint8_t *) dest + i, *((const uint8_t *) src + i), cond);
    }
    return dest;
}

void *o_memset(void *dest, unsigned char c, size_t n, bool cond) {
    for (size_t i = 0; i < n; i++) {
        o_set8((uint8_t *) dest + i, c, cond);
    }
    return dest;
}

void o_memswap(void *restrict a, void *restrict b, size_t n, bool cond) {
    for (size_t i = 0; i < n; i++) {
        o_swap8((uint8_t *) a + i, (uint8_t *) b + i, cond);
    }
}

void o_memaccess(void *restrict readp, void *restrict writep, size_t n,
        bool write, bool cond) {
    for (size_t i = 0; i < n; i++) {
        o_access8((uint8_t *) readp + i, (uint8_t *) writep + i, write, cond);
    }
}
