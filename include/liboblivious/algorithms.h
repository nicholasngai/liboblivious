#ifndef LIBOBLIVIOUS_ALGORITHMS_H
#define LIBOBLIVIOUS_ALGORITHMS_H

#include <stddef.h>
#include <stdbool.h>
#include "liboblivious/internal/defs.h"

LIBOBLIVIOUS_EXTERNC_BEGIN

void o_sort(void *data, size_t n, size_t elem_size,
        int (*comparator)(const void *a, const void *b, void *aux), void *aux);
void o_sort_generate_swaps(size_t n,
        void (*func)(size_t a, size_t b, void *aux), void *aux);

void o_compact(void *data, size_t n, size_t elem_size,
        bool (*is_marked)(const void *elem, void *aux), void *aux);
void o_compact_generate_swaps(size_t n,
        bool (*is_marked)(size_t index, void *aux),
        void (*swap)(size_t a, size_t b, bool should_swap, void *aux),
        void *aux);

LIBOBLIVIOUS_EXTERNC_END

#endif /* liboblivious/algorithms.h */
