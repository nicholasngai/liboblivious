#ifndef __LIBOBLIVIOUS_ALGORITHMS_H
#define __LIBOBLIVIOUS_ALGORITHMS_H

#include <stddef.h>
#include "liboblivious/internal/defs.h"

LIBOBLIVIOUS_EXTERNC_BEGIN

void o_sort(void *data, size_t n, size_t elem_size,
        int (*comparator)(void *a, void *b));

LIBOBLIVIOUS_EXTERNC_END

#endif /* liboblivious/algorithms.h */
