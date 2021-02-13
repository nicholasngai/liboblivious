#ifndef __LIBOBLIVIOUS_ALGORITHMS_H
#define __LIBOBLIVIOUS_ALGORITHMS_H

#include <stddef.h>

void o_sort(void *data, size_t n, size_t elem_size,
        int (*comparator)(void *a, void *b));

#endif /* liboblivious/algorithms.h */
