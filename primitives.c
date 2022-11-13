#include "liboblivious/primitives.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

void o_select(void *restrict elem, void *restrict arr_, size_t length,
        size_t elem_size, size_t index, bool write, bool cond) {
    unsigned char *restrict arr = arr_;
    for (size_t i = 0; i < length; i++) {
        o_memaccess(elem, arr + i * elem_size, elem_size, write,
                (i == index) & cond);
    }
}

void o_slice(void *restrict slice_, void *restrict arr, size_t length,
        size_t slice_start, size_t slice_length, bool write, bool cond) {
    unsigned char *restrict slice = slice_;
    for (size_t i = 0; i < slice_length; i++) {
        o_select(&slice[i], arr, length, 1, slice_start + i, write, cond);
    }
}
