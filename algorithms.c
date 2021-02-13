#include "liboblivious/algorithms.h"
#include "liboblivious/primitives.h"

void o_sort(void *data, size_t n, size_t elem_size,
        int (*comparator)(void *a, void *b)) {
    // TODO Port a better oblivious sort over. This is bubble sort for now.
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n - i - 1; j++) {
            int comp = comparator(data + i * elem_size, data + j * elem_size);
            o_memswap(data + j * elem_size, data + (j + 1) * elem_size,
                    elem_size, comp > 0);
        }
    }
}
