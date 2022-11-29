#include "liboblivious/primitives.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

void *o_memcpy(void *restrict dest_, const void *restrict src_, size_t n,
        bool cond) {
    unsigned const char *restrict src = src_;
    unsigned char *restrict dest = dest_;

    while (n >= sizeof(unsigned long long)) {
        o_setll((unsigned long long *) dest,
                *((const unsigned long long *) src), cond);
        src += sizeof(unsigned long long);
        dest += sizeof(unsigned long long);
        n -= sizeof(unsigned long long);
    }

    while (n >= sizeof(unsigned long)) {
        o_setl((unsigned long *) dest, *((const unsigned long *) src),
                cond);
        src += sizeof(unsigned long);
        dest += sizeof(unsigned long);
        n -= sizeof(unsigned long);
    }

    while (n >= sizeof(unsigned int)) {
        o_seti((unsigned int *) dest, *((const unsigned int *) src), cond);
        src += sizeof(unsigned int);
        dest += sizeof(unsigned int);
        n -= sizeof(unsigned int);
    }

    while (n >= sizeof(unsigned short)) {
        o_setshrt((unsigned short *) dest, *((const unsigned short *) src),
                cond);
        src += sizeof(unsigned short);
        dest += sizeof(unsigned short);
        n -= sizeof(unsigned short);
    }

    while (n >= sizeof(unsigned char)) {
        o_setc(dest, *src, cond);
        src += sizeof(unsigned char);
        dest += sizeof(unsigned char);
        n -= sizeof(unsigned char);
    }

    return dest;
}

void *o_memset(void *dest_, unsigned char c, size_t n, bool cond) {
    unsigned char *restrict dest = dest_;

    unsigned long long ll;
    unsigned long l;
    unsigned int i;
    unsigned short shrt;
    memset(&ll, c, sizeof(ll));
    memset(&l, c, sizeof(l));
    memset(&i, c, sizeof(i));
    memset(&shrt, c, sizeof(shrt));

    while (n >= sizeof(unsigned long long)) {
        o_setll((unsigned long long *) dest, ll, cond);
        dest += sizeof(unsigned long long);
        n -= sizeof(unsigned long long);
    }

    while (n >= sizeof(unsigned long)) {
        o_setl((unsigned long *) dest, l, cond);
        dest += sizeof(unsigned long);
        n -= sizeof(unsigned long);
    }

    while (n >= sizeof(unsigned int)) {
        o_seti((unsigned int *) dest, i, cond);
        dest += sizeof(unsigned int);
        n -= sizeof(unsigned int);
    }

    while (n >= sizeof(unsigned short)) {
        o_setshrt((unsigned short *) dest, shrt, cond);
        dest += sizeof(unsigned short);
        n -= sizeof(unsigned short);
    }

    while (n >= sizeof(unsigned char)) {
        o_setc(dest, c, cond);
        dest += sizeof(unsigned char);
        n -= sizeof(unsigned char);
    }

    return dest;
}

void o_memswap(void *restrict a_, void *restrict b_, size_t n, bool cond) {
    unsigned char *restrict a = a_;
    unsigned char *restrict b = b_;

    while (n >= sizeof(unsigned long long)) {
        o_swapll((unsigned long long *) b, (unsigned long long *) a, cond);
        a += sizeof(unsigned long long);
        b += sizeof(unsigned long long);
        n -= sizeof(unsigned long long);
    }

    while (n >= sizeof(unsigned long)) {
        o_swapl((unsigned long *) b, (unsigned long *) a, cond);
        a += sizeof(unsigned long);
        b += sizeof(unsigned long);
        n -= sizeof(unsigned long);
    }

    while (n >= sizeof(unsigned int)) {
        o_swapi((unsigned int *) b, (unsigned int *) a, cond);
        a += sizeof(unsigned int);
        b += sizeof(unsigned int);
        n -= sizeof(unsigned int);
    }

    while (n >= sizeof(unsigned short)) {
        o_swapshrt((unsigned short *) b, (unsigned short *) a, cond);
        a += sizeof(unsigned short);
        b += sizeof(unsigned short);
        n -= sizeof(unsigned short);
    }

    while (n >= sizeof(unsigned char)) {
        o_swapc(b, a, cond);
        a += sizeof(unsigned char);
        b += sizeof(unsigned char);
        n -= sizeof(unsigned char);
    }
}

void o_memaccess(void *restrict readp_, void *restrict writep_, size_t n,
        bool write, bool cond) {
    unsigned char *restrict readp = readp_;
    unsigned char *restrict writep = writep_;

    while (n >= sizeof(unsigned long long)) {
        o_accessll((unsigned long long *) readp, (unsigned long long *) writep,
                write, cond);
        readp += sizeof(unsigned long long);
        writep += sizeof(unsigned long long);
        n -= sizeof(unsigned long long);
    }

    while (n >= sizeof(unsigned long)) {
        o_accessl((unsigned long *) readp, (unsigned long *) writep, write,
                cond);
        readp += sizeof(unsigned long);
        writep += sizeof(unsigned long);
        n -= sizeof(unsigned long);
    }

    while (n >= sizeof(unsigned int)) {
        o_accessi((unsigned int *) readp, (unsigned int *) writep, write, cond);
        readp += sizeof(unsigned int);
        writep += sizeof(unsigned int);
        n -= sizeof(unsigned int);
    }

    while (n >= sizeof(unsigned short)) {
        o_accessshrt((unsigned short *) readp, (unsigned short *) writep, write,
                cond);
        readp += sizeof(unsigned short);
        writep += sizeof(unsigned short);
        n -= sizeof(unsigned short);
    }

    while (n >= sizeof(unsigned char)) {
        o_accessc(readp, writep, write, cond);
        readp += sizeof(unsigned char);
        writep += sizeof(unsigned char);
        n -= sizeof(unsigned char);
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

void o_slice(void *restrict data_, void *restrict arr, size_t data_length,
        size_t arr_length, size_t data_slice_start, size_t arr_slice_start,
        size_t slice_length, bool write, bool cond) {
    unsigned char *restrict data = data_;
    for (size_t i = 0; i < data_length; i++) {
        o_select(&data[i], arr, arr_length, 1,
                i - data_slice_start + arr_slice_start, write,
                (i >= data_slice_start) & (i < data_slice_start + slice_length)
                    & cond);
    }
}
