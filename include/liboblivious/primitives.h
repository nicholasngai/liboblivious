#ifndef LIBOBLIVIOUS_OSWAP_H
#define LIBOBLIVIOUS_OSWAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "liboblivious/internal/defs.h"

LIBOBLIVIOUS_EXTERNC_BEGIN

static inline void o_setbool(bool *dest, bool src, bool cond) {
    *dest = *dest != ((src != *dest) & cond);
}

static inline void o_set8(uint8_t *dest, uint8_t src, bool cond) {
    uint8_t mask = ~((uint8_t) cond - 1);
    *dest ^= (src ^ *dest) & mask;
}

static inline void o_set16(uint16_t *dest, uint16_t src, bool cond) {
    uint16_t mask = ~((uint16_t) cond - 1);
    *dest ^= (src ^ *dest) & mask;
}

static inline void o_set32(uint32_t *dest, uint32_t src, bool cond) {
    uint32_t mask = ~((uint32_t) cond - 1);
    *dest ^= (src ^ *dest) & mask;
}

static inline void o_set64(uint64_t *dest, uint64_t src, bool cond) {
    uint64_t mask = ~((uint64_t) cond - 1);
    *dest ^= (src ^ *dest) & mask;
}

static inline void o_swapbool(bool *restrict a, bool *restrict b, bool cond) {
    *a = *a != (*a != *b);
    *b = *b != ((*a != *b) & cond);
    *a = *a != (*a != *b);
}

static inline void o_swap8(uint8_t *restrict a, uint8_t *restrict b,
        bool cond) {
    uint8_t mask = ~((uint8_t) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
}

static inline void o_swap16(uint16_t *restrict a, uint16_t *restrict b,
        bool cond) {
    uint16_t mask = ~((uint16_t) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
}

static inline void o_swap32(uint32_t *restrict a, uint32_t *restrict b,
        bool cond) {
    uint32_t mask = ~((uint32_t) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
}

static inline void o_swap64(uint64_t *restrict a, uint64_t *restrict b,
        bool cond) {
    uint64_t mask = ~((uint64_t) cond - 1);
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

static inline void o_access8(uint8_t *restrict readp,
        uint8_t *restrict writep, bool write, bool cond) {
    uint8_t mask = ~((uint8_t) cond - 1);
    uint8_t read_mask = (uint8_t) write - 1;
    uint8_t xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
}

static inline void o_access16(uint16_t *restrict readp,
        uint16_t *restrict writep, bool write, bool cond) {
    uint16_t mask = ~((uint16_t) cond - 1);
    uint16_t read_mask = (uint16_t) write - 1;
    uint16_t xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
}

static inline void o_access32(uint32_t *restrict readp,
        uint32_t *restrict writep, bool write, bool cond) {
    uint32_t mask = ~((uint32_t) cond - 1);
    uint32_t read_mask = (uint32_t) write - 1;
    uint32_t xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
}

static inline void o_access64(uint64_t *restrict readp,
        uint64_t *restrict writep, bool write, bool cond) {
    uint64_t mask = ~((uint64_t) cond - 1);
    uint64_t read_mask = (uint64_t) write - 1;
    uint64_t xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
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

/* If COND, obliviously access the range of bytes starting at SLICE_START of
 * length SLICE_LENGTH in ARR, which is LENGTH bytes long, with SLICE, which has
 * size at least SLICE_LENGTH. The access is a write from SLICE to ARR if WRITE,
 * or else it is a read from ARR to SLICE.
 *
 * SLICE_START is kept oblivious. */
void o_slice(void *slice, void *arr, size_t length, size_t slice_start,
        size_t slice_length, bool write, bool cond);

static inline int o_min(int a, int b) {
    int ret = a;
    o_set32((unsigned int *) &ret, b, b < a);
    return ret;
}

static inline long o_minl(long a, long b) {
    long ret = a;
    o_set64((unsigned long *) &ret, b, b < a);
    return ret;
}

static inline long long o_minll(long long a, long long b) {
    long long ret = a;
    o_set64((unsigned long *) &ret, b, b < a);
    return ret;
}

static inline int o_minu(unsigned int a, unsigned int b) {
    unsigned int ret = a;
    o_set32((unsigned int *) &ret, b, b < a);
    return ret;
}

static inline long o_minul(unsigned long a, unsigned long b) {
    unsigned long ret = a;
    o_set64((unsigned long *) &ret, b, b < a);
    return ret;
}

static inline long long o_minull(unsigned long long a, unsigned long long b) {
    unsigned long long ret = a;
    o_set64((unsigned long *) &ret, b, b < a);
    return ret;
}

static inline int o_max(int a, int b) {
    int ret = a;
    o_set32((unsigned int *) &ret, b, b > a);
    return ret;
}

static inline long o_maxl(long a, long b) {
    long ret = a;
    o_set64((unsigned long *) &ret, b, b > a);
    return ret;
}

static inline long long o_maxll(long long a, long long b) {
    long long ret = a;
    o_set64((unsigned long *) &ret, b, b > a);
    return ret;
}

static inline int o_maxu(unsigned int a, unsigned int b) {
    unsigned int ret = a;
    o_set32((unsigned int *) &ret, b, b > a);
    return ret;
}

static inline long o_maxul(unsigned long a, unsigned long b) {
    unsigned long ret = a;
    o_set64((unsigned long *) &ret, b, b > a);
    return ret;
}

static inline long long o_maxull(unsigned long long a, unsigned long long b) {
    unsigned long long ret = a;
    o_set64((unsigned long *) &ret, b, b > a);
    return ret;
}

LIBOBLIVIOUS_EXTERNC_END

#endif /* liboblivious/primitives.h */
