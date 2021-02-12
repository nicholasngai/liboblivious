#include "liboblivious/primitives.h"

void *o_memcpy(void *dest, const void *src, size_t n, bool cond) {
    void *d = dest;
    const void *s = src;

    while (n >= sizeof(uint64_t)) {
        o_set64(d, *((const uint64_t *) s), cond);
        n -= sizeof(uint64_t);
        d += sizeof(uint64_t);
        s += sizeof(uint64_t);
    }

    while (n >= sizeof(uint32_t)) {
        o_set32(d, *((const uint32_t *) s), cond);
        n -= sizeof(uint32_t);
        d += sizeof(uint32_t);
        s += sizeof(uint32_t);
    }

    while (n >= sizeof(uint16_t)) {
        o_set16(dest, *((const uint16_t *) src), cond);
        n -= sizeof(uint16_t);
        d += sizeof(uint16_t);
        s += sizeof(uint16_t);
    }

    while (n >= sizeof(uint8_t)) {
        o_set8(dest, *((const uint8_t *) src), cond);
        n -= sizeof(uint8_t);
        d += sizeof(uint8_t);
        s += sizeof(uint8_t);
    }

    return dest;
}

void *o_memset(void *dest, unsigned char c, size_t n, bool cond) {
    void *d = dest;

    uint64_t c64 = (uint64_t) c << 56 | (uint64_t) c << 48 | (uint64_t) c << 40
        | (uint64_t) c << 32 | (uint64_t) c << 24 | (uint64_t) c << 16
        | (uint64_t) c << 8 | (uint64_t) c;

    while (n >= sizeof(uint64_t)) {
        o_set64(dest, c64, cond);
        n -= sizeof(uint64_t);
        d += sizeof(uint64_t);
    }

    while (n >= sizeof(uint32_t)) {
        o_set32(dest, c64, cond);
        n -= sizeof(uint32_t);
        d += sizeof(uint32_t);
    }

    while (n >= sizeof(uint16_t)) {
        o_set16(dest, c64, cond);
        n -= sizeof(uint16_t);
        d += sizeof(uint16_t);
    }

    while (n >= sizeof(uint8_t)) {
        o_set8(dest, c64, cond);
        n -= sizeof(uint8_t);
        d += sizeof(uint8_t);
    }

    return dest;
}

void o_memswap(void *a, void *b, size_t n, bool cond) {
    while (n >= sizeof(uint64_t)) {
        o_swap64((uint64_t *) a, (uint64_t *) b, cond);
        n -= sizeof(uint64_t);
        a += sizeof(uint64_t);
        b += sizeof(uint64_t);
    }

    while (n >= sizeof(uint32_t)) {
        o_swap32((uint32_t *) a, (uint32_t *) b, cond);
        n -= sizeof(uint32_t);
        a += sizeof(uint32_t);
        b += sizeof(uint32_t);
    }

    while (n >= sizeof(uint16_t)) {
        o_swap16((uint16_t *) a, (uint16_t *) b, cond);
        n -= sizeof(uint16_t);
        a += sizeof(uint16_t);
        b += sizeof(uint16_t);
    }

    while (n >= sizeof(uint8_t)) {
        o_swap8((uint8_t *) a, (uint8_t *) b, cond);
        n -= sizeof(uint8_t);
        a += sizeof(uint8_t);
        b += sizeof(uint8_t);
    }
}
