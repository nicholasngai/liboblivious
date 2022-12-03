#ifndef LIBOBLIVIOUS_OSWAP_H
#define LIBOBLIVIOUS_OSWAP_H

#include <stdbool.h>
#include <stddef.h>
#include "liboblivious/internal/defs.h"

LIBOBLIVIOUS_EXTERNC_BEGIN

static inline void o_setbool(bool *dest, bool src, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    unsigned int src_i = src;
    unsigned int dest_i = *dest;
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*dest)
            : "r,m" (src_i), "g,g" (cond)
            : "flags");
    *dest = dest_i;
#else
    *dest = *dest != ((src != *dest) & cond);
#endif
}

static inline void o_setc(unsigned char *dest, unsigned char src, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    unsigned int src_i = src;
    unsigned int dest_i = *dest;
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (dest_i)
            : "r,m" ((unsigned int) src_i), "g,g" (cond)
            : "flags");
    *dest = dest_i;
#else
    unsigned char mask = ~((unsigned char) cond - 1);
    *dest ^= (src ^ *dest) & mask;
#endif
}

static inline void o_setshrt(unsigned short *dest, unsigned short src,
        bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*dest)
            : "r,m" (src), "g,g" (cond)
            : "flags");
#else
    unsigned short mask = ~((unsigned short) cond - 1);
    *dest ^= (src ^ *dest) & mask;
#endif
}

static inline void o_seti(unsigned int *dest, unsigned int src, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*dest)
            : "r,m" (src), "g,g" (cond)
            : "flags");
#else
    unsigned int mask = ~((unsigned int) cond - 1);
    *dest ^= (src ^ *dest) & mask;
#endif
}

static inline void o_setl(unsigned long *dest, unsigned long src, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*dest)
            : "r,m" (src), "g,g" (cond)
            : "flags");
#else
    unsigned long mask = ~((unsigned long) cond - 1);
    *dest ^= (src ^ *dest) & mask;
#endif
}

static inline void o_setll(unsigned long long *dest, unsigned long long src,
        bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*dest)
            : "r,m" (src), "g,g" (cond)
            : "flags");
#else
    unsigned long mask = ~((unsigned long) cond - 1);
    *dest ^= (src ^ *dest) & mask;
#endif
}

static inline void o_swapbool(bool *restrict a, bool *restrict b, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    unsigned int a_i = *a;
    unsigned int b_i = *b;
    unsigned int temp = a_i;
    __asm__ ("cmpb $0, %3;"
            "cmova %1, %0;"
            "cmova %2, %1;"
            : "+&r,&r" (a_i), "+r,r" (b_i)
            : "r,m" (temp), "g,g" (cond)
            : "flags");
    *a = a_i;
    *b = b_i;
#else
    *a = *a != (*a != *b);
    *b = *b != ((*a != *b) & cond);
    *a = *a != (*a != *b);
#endif
}

static inline void o_swapc(unsigned char *restrict a, unsigned char *restrict b,
        bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    unsigned int a_i = *a;
    unsigned int b_i = *b;
    unsigned int temp = a_i;
    __asm__ ("cmpb $0, %3;"
            "cmova %1, %0;"
            "cmova %2, %1;"
            : "+&r,&r" (a_i), "+r,r" (b_i)
            : "r,m" (temp), "g,g" (cond)
            : "flags");
    *a = a_i;
    *b = b_i;
#else
    unsigned char mask = ~((unsigned char) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
#endif
}

static inline void o_swapshrt(unsigned short *restrict a,
        unsigned short *restrict b, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    unsigned short temp = *a;
    __asm__ ("cmpb $0, %3;"
            "cmova %1, %0;"
            "cmova %2, %1;"
            : "+&r,&r" (*a), "+r,r" (*b)
            : "r,m" (temp), "g,g" (cond)
            : "flags");
#else
    unsigned short mask = ~((unsigned short) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
#endif
}

static inline void o_swapi(unsigned int *restrict a, unsigned int *restrict b,
        bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    unsigned int temp = *a;
    __asm__ ("cmpb $0, %3;"
            "cmova %1, %0;"
            "cmova %2, %1;"
            : "+&r,&r" (*a), "+r,r" (*b)
            : "r,m" (temp), "g,g" (cond)
            : "flags");
#else
    unsigned int mask = ~((unsigned int) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
#endif
}

static inline void o_swapl(unsigned long *restrict a, unsigned long *restrict b,
        bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    unsigned long temp = *a;
    __asm__ ("cmpb $0, %3;"
            "cmova %1, %0;"
            "cmova %2, %1;"
            : "+&r,&r" (*a), "+r,r" (*b)
            : "r,m" (temp), "g,g" (cond)
            : "flags");
#else
    unsigned long mask = ~((unsigned long) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
#endif
}

static inline void o_swapll(unsigned long long *restrict a,
        unsigned long long *restrict b, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    unsigned long temp = *a;
    __asm__ ("cmpb $0, %3;"
            "cmova %1, %0;"
            "cmova %2, %1;"
            : "+&r,&r" (*a), "+r,r" (*b)
            : "r,m" (temp), "g,g" (cond)
            : "flags");
#else
    unsigned long long mask = ~((unsigned long long) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
#endif
}

static inline void o_accessbool(bool *restrict readp, bool *restrict writep,
        bool write, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    unsigned int read_i = *readp;
    unsigned int write_i = *writep;
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (read_i)
            : "r,m" (write_i), "g,g" (!write & cond)
            : "flags");
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (write_i)
            : "r,m" (read_i), "g,g" (write & cond)
            : "flags");
    *readp = read_i;
    *writep = write_i;
#else
    bool xor_val = *readp != *writep;
    *readp = *readp != (xor_val & !write & cond);
    *writep = *writep != (xor_val & write & cond);
#endif
}

static inline void o_accessc(unsigned char *restrict readp,
        unsigned char *restrict writep, bool write, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    unsigned int read_i = *readp;
    unsigned int write_i = *writep;
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (read_i)
            : "r,m" (write_i), "g,g" (!write & cond)
            : "flags");
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (write_i)
            : "r,m" (read_i), "g,g" (write & cond)
            : "flags");
    *readp = read_i;
    *writep = write_i;
#else
    unsigned char mask = ~((unsigned char) cond - 1);
    unsigned char read_mask = (unsigned char) write - 1;
    unsigned char xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
#endif
}

static inline void o_accessshrt(unsigned short *restrict readp,
        unsigned short *restrict writep, bool write, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*readp)
            : "r,m" (*writep), "g,g" (!write & cond)
            : "flags");
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*writep)
            : "r,m" (*readp), "g,g" (write & cond)
            : "flags");
#else
    unsigned short mask = ~((unsigned short) cond - 1);
    unsigned short read_mask = (unsigned short) write - 1;
    unsigned short xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
#endif
}

static inline void o_accessi(unsigned int *restrict readp,
        unsigned int *restrict writep, bool write, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*readp)
            : "r,m" (*writep), "g,g" (!write & cond)
            : "flags");
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*writep)
            : "r,m" (*readp), "g,g" (write & cond)
            : "flags");
#else
    unsigned int mask = ~((unsigned int) cond - 1);
    unsigned int read_mask = (unsigned int) write - 1;
    unsigned int xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
#endif
}

static inline void o_accessl(unsigned long *restrict readp,
        unsigned long *restrict writep, bool write, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*readp)
            : "r,m" (*writep), "g,g" (!write & cond)
            : "flags");
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*writep)
            : "r,m" (*readp), "g,g" (write & cond)
            : "flags");
#else
    unsigned long mask = ~((unsigned long) cond - 1);
    unsigned long read_mask = (unsigned long) write - 1;
    unsigned long xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
#endif
}

static inline void o_accessll(unsigned long long *restrict readp,
        unsigned long long *restrict writep, bool write, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*readp)
            : "r,m" (*writep), "g,g" (!write & cond)
            : "flags");
    __asm__ ("cmpb $0, %2;"
            "cmova %1, %0;"
            : "+r,r" (*writep)
            : "r,m" (*readp), "g,g" (write & cond)
            : "flags");
#else
    unsigned long long mask = ~((unsigned long long) cond - 1);
    unsigned long long read_mask = (unsigned long long) write - 1;
    unsigned long long xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
#endif
}

void *o_memcpy(void *dest, const void *src, size_t n, bool cond);
void *o_memset(void *dest, unsigned char src, size_t n, bool cond);
void o_memswap(void *a, void *b, size_t n, bool cond);
void o_memaccess(void *readp, void *writep, size_t n, bool write, bool cond);

/* If COND, obliviously access the item with index INDEX in SRC, which has
 * LENGTH items of size ELEM, with DEST, which has size ELEM_SIZE. The access is
 * a write from ELEM to ARR if WRITE, or else it is a read from ARR to ELEM.
 *
 * INDEX is kept oblivious. */
void o_select(void *elem, void *arr, size_t length, size_t elem_size,
        size_t index, bool write, bool cond);

/* If COND, obliviously access the range of bytes starting at ARR_SLICE_START of
 * length SLICE_LENGTH in ARR, which is ARR_LENGTH bytes long, with a range of
 * starting at DATA_SLICE_START of length SLICE_LENGTh in DATA, which is
 * DATA_LENGTH bytes long. The access is a write from SLICE to ARR if WRITE, or
 * else it is a read from ARR to SLICE. SLICE_LENGTH <= DATA_SLICE_START <=
 * ARR_SLICE_START.
 *
 * ARR_SLICE_START, DATA_SLICE_START, and SLICE_LENGTH are kept oblivious. */
void o_slice(void *data, void *arr, size_t data_length, size_t arr_length,
        size_t data_slice_start, size_t arr_slice_start, size_t slice_length,
        bool write, bool cond);

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
