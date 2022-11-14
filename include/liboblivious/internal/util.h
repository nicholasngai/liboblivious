#ifndef LIBOBLIVIOUS_INTERNAL_UTIL_H
#define LIBOBLIVIOUS_INTERNAL_UTIL_H

#include <limits.h>

#define CEIL_DIV(a, b) (((a) + (b) - 1) / (b))

static inline long ilog2(unsigned int x) {
#ifdef __GNUC__
    return sizeof(x) * CHAR_BIT - __builtin_clzl(x) - 1;
#else
    long log = -1;
    while (x) {
        log++;
        x >>= 1;
    }
    return log;
#endif
}

static inline long ilog2l(unsigned long x) {
#ifdef __GNUC__
    return sizeof(x) * CHAR_BIT - __builtin_clzl(x) - 1;
#else
    long log = -1;
    while (x) {
        log++;
        x >>= 1;
    }
    return log;
#endif
}

static inline long ilog2ll(unsigned long long x) {
#ifdef __GNUC__
    return sizeof(x) * CHAR_BIT - __builtin_clzl(x) - 1;
#else
    unsigned long long log = -1;
    while (x) {
        log++;
        x >>= 1;
    }
    return log;
#endif
}

#endif
