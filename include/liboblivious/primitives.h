#ifndef __LIBOBLIVIOUS_OSWAP_H
#define __LIBOBLIVIOUS_OSWAP_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static inline void o_set8(uint8_t *dest, uint8_t src, bool cond) {
    uint8_t mask = ~((uint8_t) cond - 1);
    *dest = (src ^ *dest) & mask;
}

static inline void o_set16(uint16_t *dest, uint16_t src, bool cond) {
    uint16_t mask = ~((uint16_t) cond - 1);
    *dest = (src ^ *dest) & mask;
}

static inline void o_set32(uint32_t *dest, uint32_t src, bool cond) {
    uint32_t mask = ~((uint32_t) cond - 1);
    *dest = (src ^ *dest) & mask;
}

static inline void o_set64(uint64_t *dest, uint64_t src, bool cond) {
    uint64_t mask = ~((uint64_t) cond - 1);
    *dest = (src ^ *dest) & mask;
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

void *o_memcpy(void *dest, const void *src, size_t n, bool cond);
void *o_memset(void *dest, unsigned char src, size_t n, bool cond);
void o_memswap(void *a, void *b, size_t n, bool cond);

#endif /* liboblivious/o_swap.h */
