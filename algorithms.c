#include "liboblivious/algorithms.h"
#include "liboblivious/primitives.h"

/* Sorting. */

static void merge_slice(size_t start, size_t n, size_t skip, bool right_heavy,
        void (*func)(size_t a, size_t b, void *aux), void *aux) {
    switch (n) {
        case 0:
        case 1:
            /* Do nothing. */
            break;
        case 2: {
            func(start, start + skip, aux);
            break;
        }
        default: {
            /* Even slices are right-heavy iff the even slice has an odd length
             * and the current slice is right-heavy. The short-circuit operator
             * is fine because the whole sort is determinstic. */
            size_t even_length = (n + 1) / 2;
            bool even_right_heavy = even_length % 2 == 1 && right_heavy;
            merge_slice(start, even_length, skip * 2, even_right_heavy, func,
                    aux);
            /* Odd slices are right-heavy iff the odd slice has an odd length
             * and either the current slice is right-heavy or the current slice
             * has an even length. Again, the short-circuit operator is fine
             * because it will be deterministic. */
            size_t odd_length = n / 2;
            bool odd_right_heavy = odd_length % 2 == 1
                && (right_heavy || n % 2 == 0);
            merge_slice(start + skip, odd_length, skip * 2, odd_right_heavy,
                    func, aux);

            /* Sort adjacent pairs such that one pair crosses the left-right
             * boundary, which depends on whether the sorted list is
             * right-heavy. If the left sorted half has an even length, then we
             * start at 1; otherweise, we start at 0. We compute this by taking
             * half the total length, and the left half will have an extra
             * member if the total length is odd, and we are not right-heavy.
             * The short-circuit operator is deterministic. Taking this mod 2
             * and then inverting it by subtracting it from 1 gives the
             * starting index. */
            for (size_t i = 1 - (n / 2 + (n % 2 == 1 && !right_heavy)) % 2;
                    i < n - 1; i += 2) {
                func(start + skip * i, start + skip * (i + 1), aux);
            }
            break;
         }
    }
}

static void sort_slice(size_t start, size_t n, size_t skip,
        void (*func)(size_t a, size_t b, void *aux), void *aux) {
    switch (n) {
        case 0:
        case 1:
            /* Do nothing. */
            break;
        case 2: {
            func(start, start + skip, aux);
            break;
        }
        default: {
            /* Sort left and right halves. Sorting doesn't care if it's
             * right-heavy. */
            size_t left_length = (n + 1) / 2;
            size_t right_length = n / 2;
            sort_slice(start, left_length, skip, func, aux);
            sort_slice(start + skip * left_length, right_length, skip, func,
                    aux);

            /* Odd-even merge. */
            merge_slice(start, n, skip, false, func, aux);
            break;
        }
    }
}

void o_sort_generate_swaps(size_t n,
        void (*func)(size_t a, size_t b, void *aux), void *aux) {
    /* This is an implementation of Batcher's odd-even mergesort, with an
     * additional right-heavy flag to work for arbitrary-sized arrays, not just
     * powers of 2. */
    sort_slice(0, n, 1, func, aux);
}

struct sort_swap_aux {
    unsigned char *data;
    size_t elem_size;
    int (*comparator)(const void *a, const void *b, void *aux);
    void *aux;
};
static void sort_swap(size_t a, size_t b, void *aux_) {
    struct sort_swap_aux *aux = aux_;
    void *a_addr = aux->data + aux->elem_size * a;
    void *b_addr = aux->data + aux->elem_size * b;
    int comp = aux->comparator(a_addr, b_addr, aux->aux);
    o_memswap(a_addr, b_addr, aux->elem_size, comp > 0);
}

void o_sort(void *data, size_t n, size_t elem_size,
        int (*comparator)(const void *a, const void *b, void *aux), void *aux) {
    struct sort_swap_aux sort_swap_aux = {
        .data = data,
        .elem_size = elem_size,
        .comparator = comparator,
        .aux = aux,
    };
    o_sort_generate_swaps(n, sort_swap, &sort_swap_aux);
}
