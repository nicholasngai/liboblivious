#ifndef LIBOBLIVIOUS_OSWAP_H
#define LIBOBLIVIOUS_OSWAP_H

#include <stdbool.h>
#include <stddef.h>
#include "liboblivious/internal/defs.h"

LIBOBLIVIOUS_EXTERNC_BEGIN

static inline void o_setbool(bool *dest, bool src, bool cond) {
    *dest = *dest != ((src != *dest) & cond);
}

static inline void o_setc(unsigned char *dest, unsigned char src, bool cond) {
    unsigned char mask = ~((unsigned char) cond - 1);
    *dest ^= (src ^ *dest) & mask;
}

static inline void o_setshrt(unsigned short *dest, unsigned short src,
        bool cond) {
    unsigned short mask = ~((unsigned short) cond - 1);
    *dest ^= (src ^ *dest) & mask;
}

static inline void o_seti(unsigned int *dest, unsigned int src, bool cond) {
    unsigned int mask = ~((unsigned int) cond - 1);
    *dest ^= (src ^ *dest) & mask;
}

static inline void o_setl(unsigned long *dest, unsigned long src, bool cond) {
    unsigned long mask = ~((unsigned long) cond - 1);
    *dest ^= (src ^ *dest) & mask;
}

static inline void o_setll(unsigned long long *dest, unsigned long long src,
        bool cond) {
    unsigned long mask = ~((unsigned long) cond - 1);
    *dest ^= (src ^ *dest) & mask;
}

static inline void o_swapbool(bool *restrict a, bool *restrict b, bool cond) {
    *a = *a != (*a != *b);
    *b = *b != ((*a != *b) & cond);
    *a = *a != (*a != *b);
}

static inline void o_swapc(unsigned char *restrict a, unsigned char *restrict b,
        bool cond) {
    unsigned char mask = ~((unsigned char) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
}

static inline void o_swapshrt(unsigned short *restrict a,
        unsigned short *restrict b, bool cond) {
    unsigned short mask = ~((unsigned short) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
}

static inline void o_swapi(unsigned int *restrict a, unsigned int *restrict b,
        bool cond) {
    unsigned int mask = ~((unsigned int) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
}

static inline void o_swapl(unsigned long *restrict a, unsigned long *restrict b,
        bool cond) {
    unsigned long mask = ~((unsigned long) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
}

static inline void o_swapll(unsigned long long *restrict a,
        unsigned long long *restrict b, bool cond) {
    unsigned long long mask = ~((unsigned long long) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
}

static inline void o_accessbool(bool *restrict readp, bool *restrict writep,
        bool write, bool cond) {
    bool xor_val = *readp != *writep;
    *readp = *readp != (xor_val & !write & cond);
    *writep = *writep != (xor_val & write & cond);
}

static inline void o_accessc(unsigned char *restrict readp,
        unsigned char *restrict writep, bool write, bool cond) {
    unsigned char mask = ~((unsigned char) cond - 1);
    unsigned char read_mask = (unsigned char) write - 1;
    unsigned char xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
}

static inline void o_accessshrt(unsigned short *restrict readp,
        unsigned short *restrict writep, bool write, bool cond) {
    unsigned short mask = ~((unsigned short) cond - 1);
    unsigned short read_mask = (unsigned short) write - 1;
    unsigned short xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
}

static inline void o_accessi(unsigned int *restrict readp,
        unsigned int *restrict writep, bool write, bool cond) {
    unsigned int mask = ~((unsigned int) cond - 1);
    unsigned int read_mask = (unsigned int) write - 1;
    unsigned int xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
}

static inline void o_accessl(unsigned long *restrict readp,
        unsigned long *restrict writep, bool write, bool cond) {
    unsigned long mask = ~((unsigned long) cond - 1);
    unsigned long read_mask = (unsigned long) write - 1;
    unsigned long xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
}

static inline void o_accessll(unsigned long long *restrict readp,
        unsigned long long *restrict writep, bool write, bool cond) {
    unsigned long long mask = ~((unsigned long long) cond - 1);
    unsigned long long read_mask = (unsigned long long) write - 1;
    unsigned long long xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
}

static inline void *o_memcpy(void *restrict dest_, const void *restrict src_,
        size_t n, bool cond) {
    unsigned const char *restrict src = src_;
    unsigned char *restrict dest = dest_;
    for (size_t i = 0; i < n; i++) {
        o_setc(&dest[i], src[i], cond);
    }
    return dest;
}

static inline void *o_memset(void *dest_, unsigned char c, size_t n,
        bool cond) {
    unsigned char *restrict dest = dest_;
    for (size_t i = 0; i < n; i++) {
        o_setc(&dest[i], c, cond);
    }
    return dest;
}

static inline void o_memswap(void *restrict a_, void *restrict b_, size_t n,
        bool cond) {
    unsigned char *restrict a = a_;
    unsigned char *restrict b = b_;
    for (size_t i = 0; i < n; i++) {
        o_swapc(&a[i], &b[i], cond);
    }
}

static inline void o_memaccess(void *restrict readp_, void *restrict writep_,
        size_t n, bool write, bool cond) {
    unsigned char *restrict readp = readp_;
    unsigned char *restrict writep = writep_;
    for (size_t i = 0; i < n; i++) {
        o_accessc(&readp[i], &writep[i], write, cond);
    }
}

/* If COND, obliviously access the item with index INDEX in SRC, which has
 * LENGTH items of size ELEM, with DEST, which has size ELEM_SIZE. The access is
 * a write from ELEM to ARR if WRITE, or else it is a read from ARR to ELEM.
 *
 * INDEX is kept oblivious. */
static inline void o_select(void *restrict elem, void *restrict arr_, size_t length,
        size_t elem_size, size_t index, bool write, bool cond) {
    unsigned char *restrict arr = arr_;
    for (size_t i = 0; i < length; i++) {
        o_memaccess(elem, arr + i * elem_size, elem_size, write,
                (i == index) & cond);
    }
}

/* If COND, obliviously access the range of bytes starting at ARR_SLICE_START of
 * length SLICE_LENGTH in ARR, which is ARR_LENGTH bytes long, with a range of
 * starting at DATA_SLICE_START of length SLICE_LENGTh in DATA, which is
 * DATA_LENGTH bytes long. The access is a write from SLICE to ARR if WRITE, or
 * else it is a read from ARR to SLICE. SLICE_LENGTH <= DATA_SLICE_START <=
 * ARR_SLICE_START.
 *
 * ARR_SLICE_START, DATA_SLICE_START, and SLICE_LENGTH are kept oblivious. */
static inline void o_slice(void *restrict data_, void *restrict arr,
        size_t data_length, size_t arr_length, size_t data_slice_start,
        size_t arr_slice_start, size_t slice_length, bool write, bool cond) {
    unsigned char *restrict data = data_;
    for (size_t i = 0; i < data_length; i++) {
        o_select(&data[i], arr, arr_length, 1,
                i - data_slice_start + arr_slice_start, write,
                (i >= data_slice_start) & (i < data_slice_start + slice_length)
                    & cond);
    }
}

static inline int o_min(int a, int b) {
    int ret = a;
    o_seti((unsigned int *) &ret, b, b < a);
    return ret;
}

static inline long o_minl(long a, long b) {
    long ret = a;
    o_setl((unsigned long *) &ret, b, b < a);
    return ret;
}

static inline long long o_minll(long long a, long long b) {
    long long ret = a;
    o_setll((unsigned long long *) &ret, b, b < a);
    return ret;
}

static inline int o_minu(unsigned int a, unsigned int b) {
    unsigned int ret = a;
    o_seti(&ret, b, b < a);
    return ret;
}

static inline long o_minul(unsigned long a, unsigned long b) {
    unsigned long ret = a;
    o_setl(&ret, b, b < a);
    return ret;
}

static inline long long o_minull(unsigned long long a, unsigned long long b) {
    unsigned long long ret = a;
    o_setll(&ret, b, b < a);
    return ret;
}

static inline int o_max(int a, int b) {
    int ret = a;
    o_seti((unsigned int *) &ret, b, b > a);
    return ret;
}

static inline long o_maxl(long a, long b) {
    long ret = a;
    o_setl((unsigned long *) &ret, b, b > a);
    return ret;
}

static inline long long o_maxll(long long a, long long b) {
    long long ret = a;
    o_setll((unsigned long long *) &ret, b, b > a);
    return ret;
}

static inline int o_maxu(unsigned int a, unsigned int b) {
    unsigned int ret = a;
    o_seti(&ret, b, b > a);
    return ret;
}

static inline long o_maxul(unsigned long a, unsigned long b) {
    unsigned long ret = a;
    o_setl(&ret, b, b > a);
    return ret;
}

static inline long long o_maxull(unsigned long long a, unsigned long long b) {
    unsigned long long ret = a;
    o_setll(&ret, b, b > a);
    return ret;
}

LIBOBLIVIOUS_EXTERNC_END

#endif /* liboblivious/primitives.h */
