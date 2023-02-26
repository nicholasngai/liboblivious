#ifndef LIBOBLIVIOUS_OSWAP_H
#define LIBOBLIVIOUS_OSWAP_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "liboblivious/internal/defs.h"

LIBOBLIVIOUS_EXTERNC_BEGIN

static inline void o_setbool(bool *dest, bool src, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    unsigned int src_i = src;
    unsigned int dest_i = *dest;
    __asm__ ("cmpb $0, %2;"
            "cmovnz %1, %0;"
            : "+r" (dest_i)
            : "rm" (src_i), "rm" (cond)
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
            "cmovnz %1, %0;"
            : "+r" (dest_i)
            : "rm" ((unsigned int) src_i), "rm" (cond)
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
            "cmovnz %1, %0;"
            : "+r" (*dest)
            : "rm" (src), "rm" (cond)
            : "memory", "flags");
#else
    unsigned short mask = ~((unsigned short) cond - 1);
    *dest ^= (src ^ *dest) & mask;
#endif
}

static inline void o_seti(unsigned int *dest, unsigned int src, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    __asm__ ("cmpb $0, %2;"
            "cmovnz %1, %0;"
            : "+r" (*dest)
            : "rm" (src), "rm" (cond)
            : "flags");
#else
    unsigned int mask = ~((unsigned int) cond - 1);
    *dest ^= (src ^ *dest) & mask;
#endif
}

static inline void o_setl(unsigned long *dest, unsigned long src, bool cond) {
#ifdef LIBOBLIVIOUS_CMOV
    __asm__ ("cmpb $0, %2;"
            "cmovnz %1, %0;"
            : "+r" (*dest)
            : "rm" (src), "rm" (cond)
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
            "cmovnz %1, %0;"
            : "+r" (*dest)
            : "rm" (src), "rm" (cond)
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
            "cmovnz %1, %0;"
            "cmovnz %2, %1;"
            : "+&r" (a_i), "+r" (b_i)
            : "rm" (temp), "rm" (cond)
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
            "cmovnz %1, %0;"
            "cmovnz %2, %1;"
            : "+&r" (a_i), "+r" (b_i)
            : "rm" (temp), "rm" (cond)
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
            "cmovnz %1, %0;"
            "cmovnz %2, %1;"
            : "+&r" (*a), "+r" (*b)
            : "rm" (temp), "rm" (cond)
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
            "cmovnz %1, %0;"
            "cmovnz %2, %1;"
            : "+&r" (*a), "+r" (*b)
            : "rm" (temp), "rm" (cond)
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
            "cmovnz %1, %0;"
            "cmovnz %2, %1;"
            : "+&r" (*a), "+r" (*b)
            : "rm" (temp), "rm" (cond)
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
            "cmovnz %1, %0;"
            "cmovnz %2, %1;"
            : "+&r" (*a), "+r" (*b)
            : "rm" (temp), "rm" (cond)
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
    __asm__ ("cmpb %3, %2;"
            "cmovl %1, %0;"
            "testb %3, %2;"
            "cmovnz %0, %1;"
            : "+r,r" (read_i), "+r,r" (write_i)
            : "r,rm" (write), "rm,r" (cond)
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
    __asm__ ("cmpb %3, %2;"
            "cmovl %1, %0;"
            "testb %3, %2;"
            "cmovnz %0, %1;"
            : "+r,r" (read_i), "+r,r" (write_i)
            : "r,rm" (write), "rm,r" (cond)
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
    __asm__ ("cmpb %3, %2;"
            "cmovl %1, %0;"
            "testb %3, %2;"
            "cmovnz %0, %1;"
            : "+r,r" (readp), "+r,r" (writep)
            : "r,rm" (write), "rm,r" (cond)
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
    __asm__ ("cmpb %3, %2;"
            "cmovl %1, %0;"
            "testb %3, %2;"
            "cmovnz %0, %1;"
            : "+r,r" (readp), "+r,r" (writep)
            : "r,rm" (write), "rm,r" (cond)
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
    __asm__ ("cmpb %3, %2;"
            "cmovl %1, %0;"
            "testb %3, %2;"
            "cmovnz %0, %1;"
            : "+r,r" (readp), "+r,r" (writep)
            : "r,rm" (write), "rm,r" (cond)
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
    __asm__ ("cmpb %3, %2;"
            "cmovl %1, %0;"
            "testb %3, %2;"
            "cmovnz %0, %1;"
            : "+r,r" (readp), "+r,r" (writep)
            : "r,rm" (write), "rm,r" (cond)
            : "flags");
#else
    unsigned long long mask = ~((unsigned long long) cond - 1);
    unsigned long long read_mask = (unsigned long long) write - 1;
    unsigned long long xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
#endif
}

static inline void *o_memcpy(void *restrict dest_, const void *restrict src_,
        size_t n, bool cond) {
    unsigned const char *restrict src = src_;
    unsigned char *restrict dest = dest_;

#ifdef LIBOBLIVIOUS_CMOV
    while (n % sizeof(unsigned short)) {
        o_setc(dest, *src, cond);
        src += sizeof(unsigned char);
        dest += sizeof(unsigned char);
        n -= sizeof(unsigned char);
    }

    while (n % sizeof(unsigned int)) {
        o_setshrt((unsigned short *) dest, *((const unsigned short *) src),
                cond);
        src += sizeof(unsigned short);
        dest += sizeof(unsigned short);
        n -= sizeof(unsigned short);
    }

    while (n % sizeof(unsigned long)) {
        o_seti((unsigned int *) dest, *((const unsigned int *) src), cond);
        src += sizeof(unsigned int);
        dest += sizeof(unsigned int);
        n -= sizeof(unsigned int);
    }

    while (n / sizeof(unsigned long) % 4) {
        o_setl((unsigned long *) dest, *((const unsigned long *) src), cond);
        src += sizeof(unsigned long);
        dest += sizeof(unsigned long);
        n -= sizeof(unsigned long);
    }

    if (n > 0) {
        register unsigned long t0;
        register unsigned long t1;
        register unsigned long t2;
        register unsigned long t3;
        __asm__ __volatile__ ("0:"
                "cmpb $0, %7;"
                "mov (%0), %3;"
                "mov %c8(%0), %4;"
                "mov %c9(%0), %5;"
                "mov %c10(%0), %6;"
                "cmovnz (%1), %3;"
                "cmovnz %c8(%1), %4;"
                "cmovnz %c9(%1), %5;"
                "cmovnz %c10(%1), %6;"
                "mov %3, (%0);"
                "mov %4, %c8(%0);"
                "mov %5, %c9(%0);"
                "mov %6, %c10(%0);"
                "add %11, %0;"
                "add %11, %1;"
                "sub %11, %2;"
                "jnz 0b;"
                : "+r" (dest), "+r" (src), "+rm" (n), "=&r" (t0), "=&r" (t1),
                    "=&r" (t2), "=&r" (t3)
                : "rm" (cond), "i" (sizeof(unsigned long)),
                    "i" (sizeof(unsigned long) * 2),
                    "i" (sizeof(unsigned long) * 3),
                    "i" (sizeof(unsigned long) * 4)
                : "memory", "flags");
    }
#else
    for (size_t i = 0; i < n; i++) {
        o_setc(&dest[i], src[i], cond);
    }
#endif

    return dest_;
}

static inline void *o_memset(void *dest_, unsigned char c, size_t n,
        bool cond) {
    unsigned char *restrict dest = dest_;

#ifdef LIBOBLIVIOUS_CMOV
    unsigned long cl;
    memset(&cl, c, sizeof(cl));

    while (n % sizeof(unsigned short)) {
        o_setc(dest, cl, cond);
        dest += sizeof(unsigned char);
        n -= sizeof(unsigned char);
    }

    while (n % sizeof(unsigned int)) {
        o_setshrt((unsigned short *) dest, cl, cond);
        dest += sizeof(unsigned short);
        n -= sizeof(unsigned short);
    }

    while (n % sizeof(unsigned long)) {
        o_seti((unsigned int *) dest, cl, cond);
        dest += sizeof(unsigned int);
        n -= sizeof(unsigned int);
    }

    while (n / sizeof(unsigned long) % 4) {
        o_setl((unsigned long *) dest, cl, cond);
        dest += sizeof(unsigned long);
        n -= sizeof(unsigned long);
    }

    if (n > 0) {
        register unsigned long t0;
        register unsigned long t1;
        register unsigned long t2;
        register unsigned long t3;
        __asm__ __volatile__ ("0:"
                "cmpb $0, %7;"
                "mov (%0), %2;"
                "mov %c8(%0), %3;"
                "mov %c9(%0), %4;"
                "mov %c10(%0), %5;"
                "cmovnz %6, %2;"
                "cmovnz %6, %3;"
                "cmovnz %6, %4;"
                "cmovnz %6, %5;"
                "mov %2, (%0);"
                "mov %3, %c8(%0);"
                "mov %4, %c9(%0);"
                "mov %5, %c10(%0);"
                "add %11, %0;"
                "sub %11, %1;"
                "jnz 0b;"
                : "+r" (dest), "+rm" (n), "=&r" (t0), "=&r" (t1), "=&r" (t2),
                    "=&r" (t3)
                : "r" (cl), "rm" (cond), "i" (sizeof(unsigned long)),
                    "i" (sizeof(unsigned long) * 2),
                    "i" (sizeof(unsigned long) * 3),
                    "i" (sizeof(unsigned long) * 4)
                : "memory", "flags");
    }
#else
    for (size_t i = 0; i < n; i++) {
        o_setc(&dest[i], c, cond);
    }
#endif

    return dest_;
}

static inline void o_memswap(void *restrict a_, void *restrict b_, size_t n,
        bool cond) {
    unsigned char *restrict a = a_;
    unsigned char *restrict b = b_;

#ifdef LIBOBLIVIOUS_CMOV
    while (n % sizeof(unsigned short)) {
        o_swapc(a, b, cond);
        a += sizeof(unsigned char);
        b += sizeof(unsigned char);
        n -= sizeof(unsigned char);
    }

    while (n % sizeof(unsigned int)) {
        o_swapshrt((unsigned short *) a, (unsigned short *) b, cond);
        a += sizeof(unsigned short);
        b += sizeof(unsigned short);
        n -= sizeof(unsigned short);
    }

    while (n % sizeof(unsigned long)) {
        o_swapi((unsigned int *) a, (unsigned int *) b, cond);
        a += sizeof(unsigned int);
        b += sizeof(unsigned int);
        n -= sizeof(unsigned int);
    }

    while (n / sizeof(unsigned long) % 2) {
        o_swapl((unsigned long *) a, (unsigned long *) b, cond);
        a += sizeof(unsigned long);
        b += sizeof(unsigned long);
        n -= sizeof(unsigned long);
    }

    if (n) {
        register unsigned long t0;
        register unsigned long t1;
        register unsigned long t2;
        register unsigned long t3;
        __asm__ __volatile__ ("0:"
                "cmpb $0, %7;"
                "mov (%0), %3;"
                "mov %c8(%0), %4;"
                "mov (%1), %5;"
                "mov %c8(%1), %6;"
                "cmovnz (%1), %3;"
                "cmovnz %c8(%1), %4;"
                "cmovnz (%0), %5;"
                "cmovnz %c8(%0), %6;"
                "mov %3, (%0);"
                "mov %4, %c8(%0);"
                "mov %5, (%1);"
                "mov %6, %c8(%1);"
                "add %9, %0;"
                "add %9, %1;"
                "sub %9, %2;"
                "jnz 0b;"
                : "+r" (a), "+r" (b), "+rm" (n), "=&r" (t0), "=&r" (t1), "=&r" (t2),
                    "=&r" (t3)
                : "rm" (cond), "i" (sizeof(unsigned long)),
                    "i" (sizeof(unsigned long) * 2)
                : "memory", "flags");
    }
#else
    for (size_t i = 0; i < n; i++) {
        o_swapc(&a[i], &b[i], cond);
    }
#endif
}

static inline void o_memaccess(void *restrict readp_, void *restrict writep_,
        size_t n, bool write, bool cond) {
    unsigned char *restrict readp = readp_;
    unsigned char *restrict writep = writep_;

#ifdef LIBOBLIVIOUS_CMOV
    while (n % sizeof(unsigned short)) {
        o_accessc(readp, writep, write, cond);
        readp += sizeof(unsigned char);
        writep += sizeof(unsigned char);
        n -= sizeof(unsigned char);
    }

    while (n % sizeof(unsigned int)) {
        o_accessshrt((unsigned short *) readp, (unsigned short *) writep, write,
                cond);
        readp += sizeof(unsigned short);
        writep += sizeof(unsigned short);
        n -= sizeof(unsigned short);
    }

    while (n % sizeof(unsigned long)) {
        o_accessi((unsigned int *) readp, (unsigned int *) writep, write, cond);
        readp += sizeof(unsigned int);
        writep += sizeof(unsigned int);
        n -= sizeof(unsigned int);
    }

    while (n / sizeof(unsigned long) % 2) {
        o_accessl((unsigned long *) readp, (unsigned long *) writep, write,
                cond);
        readp += sizeof(unsigned long);
        writep += sizeof(unsigned long);
        n -= sizeof(unsigned long);
    }

    if (n) {
#ifdef __x86_64__
        register unsigned long t0;
        register unsigned long t1;
        register unsigned long t2;
        register unsigned long t3;
        __asm__ __volatile__ ("0:"
                "mov (%0), %3;"
                "mov %c9(%0), %4;"
                "mov (%1), %5;"
                "mov %c9(%1), %6;"
                "cmp %8, %7;"
                "cmovl %5, %3;"
                "cmovl %6, %4;"
                "test %8, %7;"
                "cmovnz %3, %5;"
                "cmovnz %4, %6;"
                "mov %3, (%0);"
                "mov %4, %c9(%0);"
                "mov %5, (%1);"
                "mov %6, %c9(%1);"
                "add %10, %0;"
                "add %10, %1;"
                "sub %10, %2;"
                "jnz 0b;"
                : "+r" (readp), "+r" (writep), "+r" (n), "=&r" (t0), "=&r" (t1),
                    "=&r" (t2), "=&r" (t3)
                : "rm" ((unsigned char) write), "rm" ((unsigned char) cond),
                    "i" (sizeof(unsigned long)), "i" (sizeof(unsigned long) * 2)
                : "memory", "flags");
#else
        register unsigned long t0;
        register unsigned long t1;
        __asm__ __volatile__ ("0:"
                "mov (%0), %3;"
                "mov (%1), %4;"
                "cmp %6, %5;"
                "cmovl %4, %3;"
                "test %6, %5;"
                "cmovnz %3, %4;"
                "mov %3, (%0);"
                "mov %4, (%1);"
                "add %7, %0;"
                "add %7, %1;"
                "sub %7, %2;"
                "jnz 0b;"
                : "+r" (readp), "+r" (writep), "+r" (n),
                    "=&r" (t0), "=&r" (t1)
                : "rm" ((unsigned char) write), "rm" ((unsigned char) cond),
                    "i" (sizeof(unsigned long))
                : "memory", "flags");
#endif
    }
#else
    for (size_t i = 0; i < n; i++) {
        o_accessc(&readp[i], &writep[i], write, cond);
    }
#endif
}

/* If COND, obliviously access the item with index INDEX in SRC, which has
 * LENGTH items of size ELEM, with DEST, which has size ELEM_SIZE. The access is
 * a write from ELEM to ARR if WRITE, or else it is a read from ARR to ELEM.
 *
 * INDEX is kept oblivious. */
static inline void o_select(void *restrict elem, void *restrict arr_,
        size_t length, size_t elem_size, size_t index, bool write, bool cond) {
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
