#ifndef __LIBOBLIVIOUS_OSWAP_H
#define __LIBOBLIVIOUS_OSWAP_H

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

static inline void o_swapbool(bool *a, bool *b, bool cond) {
    *a = *a != (*a != *b);
    *b = *b != ((*a != *b) & cond);
    *a = *a != (*a != *b);
}

static inline void o_swap8(uint8_t *a, uint8_t *b, bool cond) {
    uint8_t mask = ~((uint8_t) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
}

static inline void o_swap16(uint16_t *a, uint16_t *b, bool cond) {
    uint16_t mask = ~((uint16_t) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
}

static inline void o_swap32(uint32_t *a, uint32_t *b, bool cond) {
    uint32_t mask = ~((uint32_t) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
}

static inline void o_swap64(uint64_t *a, uint64_t *b, bool cond) {
    uint64_t mask = ~((uint64_t) cond - 1);
    *a ^= *b;
    *b ^= *a & mask;
    *a ^= *b;
}

static inline void o_accessbool(bool *readp, bool *writep, bool write,
        bool cond) {
    bool xor_val = *readp != *writep;
    *readp = *readp != (xor_val & !write & cond);
    *writep = *writep != (xor_val & write & cond);
}

static inline void o_access8(uint8_t *readp, uint8_t *writep, bool write,
        bool cond) {
    uint8_t mask = ~((uint8_t) cond - 1);
    uint8_t read_mask = (uint8_t) write - 1;
    uint8_t xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
}

static inline void o_access16(uint16_t *readp, uint16_t *writep, bool write,
        bool cond) {
    uint16_t mask = ~((uint16_t) cond - 1);
    uint16_t read_mask = (uint16_t) write - 1;
    uint16_t xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
}

static inline void o_access32(uint32_t *readp, uint32_t *writep, bool write,
        bool cond) {
    uint32_t mask = ~((uint32_t) cond - 1);
    uint32_t read_mask = (uint32_t) write - 1;
    uint32_t xor_val = *readp ^ *writep;
    *readp ^= xor_val & read_mask & mask;
    *writep ^= xor_val & ~read_mask & mask;
}

static inline void o_access64(uint64_t *readp, uint64_t *writep, bool write,
        bool cond) {
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

LIBOBLIVIOUS_EXTERNC_END

#endif /* liboblivious/primitives.h */
