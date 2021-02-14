#include "liboblivious/algorithms.h"
#include "liboblivious/primitives.h"

void o_sort(void *data, size_t n, size_t elem_size,
        int (*comparator)(void *a, void *b)) {
    // TODO Port a better oblivious sort over. This is bubble sort for now.
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n - i - 1; j++) {
            size_t a = j * elem_size;
            size_t b = (j + 1) * elem_size;
            int comp = comparator(data + a, data + b);
            o_memswap(data + a, data + b, elem_size, comp > 0);
        }
    }
}
