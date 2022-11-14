#include "liboblivious/primitives.h"
#include <stdbool.h>
#include <stddef.h>

void *o_memcpy(void *restrict dest_, const void *restrict src_, size_t n,
        bool cond) {
    unsigned const char *restrict src = src_;
    unsigned char *restrict dest = dest_;
    for (size_t i = 0; i < n; i++) {
        o_setc(&dest[i], src[i], cond);
    }
    return dest;
}

void *o_memset(void *dest_, unsigned char c, size_t n, bool cond) {
    unsigned char *restrict dest = dest_;
    for (size_t i = 0; i < n; i++) {
        o_setc(&dest[i], c, cond);
    }
    return dest;
}

void o_memswap(void *restrict a_, void *restrict b_, size_t n, bool cond) {
    unsigned char *restrict a = a_;
    unsigned char *restrict b = b_;
    for (size_t i = 0; i < n; i++) {
        o_swapc(&a[i], &b[i], cond);
    }
}

void o_memaccess(void *restrict readp_, void *restrict writep_, size_t n,
        bool write, bool cond) {
    unsigned char *restrict readp = readp_;
    unsigned char *restrict writep = writep_;
    for (size_t i = 0; i < n; i++) {
        o_accessc(&readp[i], &writep[i], write, cond);
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
