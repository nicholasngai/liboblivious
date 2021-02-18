#include "liboblivious/algorithms.h"
#include "liboblivious/primitives.h"

static void sort_swap(void *a, void *b, size_t elem_size,
        int (*comparator)(void *a, void *b));
static void sort_slice(void *data, size_t n, size_t elem_size, size_t skip,
        bool right_heavy, int (*comparator)(void *a, void *b));
static void merge_slice(void *data, size_t n, size_t elem_size, size_t skip,
        bool right_heavy, int (*comparator)(void *a, void *b));

void o_sort(void *data, size_t n, size_t elem_size,
        int (*comparator)(void *a, void *b)) {
    /* This is an implementation of Batcher's odd-even mergesort, with an
     * additional right-heavy flag to work for arbitrary-sized arrays, not just
     * powers of 2. */
    sort_slice(data, n, elem_size, 1, false, comparator);
}

static void sort_swap(void *a, void *b, size_t elem_size,
        int (*comparator)(void *a, void *b)) {
    int comp = comparator(a, b);
    o_memswap(a, b, elem_size, comp > 0);
}

static void sort_slice(void *data, size_t n, size_t elem_size, size_t skip,
        bool right_heavy, int (*comparator)(void *a, void *b)) {
    switch (n) {
        case 0:
        case 1:
            /* Do nothing. */
            break;
        case 2: {
            sort_swap(data, data + elem_size * skip, elem_size, comparator);
            break;
        }
        default: {
            /* Sort left and right halves. Sorting doesn't care if it's
             * right-heavy. */
            size_t left_length = (n + 1) / 2;
            size_t right_length = n / 2;
            sort_slice(data, left_length, elem_size, skip, false, comparator);
            sort_slice(data + elem_size * skip * left_length, right_length,
                    elem_size, skip, false, comparator);

            /* Odd-even merge. */
            merge_slice(data, n, elem_size, skip, right_heavy, comparator);
            break;
        }
    }
}

static void merge_slice(void *data, size_t n, size_t elem_size, size_t skip,
        bool right_heavy, int (*comparator)(void *a, void *b)) {
    switch (n) {
        case 0:
        case 1:
            /* Do nothing. */
            break;
        case 2: {
            sort_swap(data, data + elem_size * skip, elem_size, comparator);
            break;
        }
        default: {
            /* Even slices are right-heavy iff the even slice has an odd length
             * and the current slice is right-heavy. The short-circuit operator
             * is fine because the whole sort is determinstic. */
            size_t even_length = (n + 1) / 2;
            bool even_right_heavy = even_length % 2 == 1 && right_heavy;
            merge_slice(data, even_length, elem_size, skip * 2,
                    even_right_heavy, comparator);
            /* Odd slices are right-heavy iff the odd slice has an odd length
             * and either the current slice is right-heavy or the current slice
             * has an even length. Again, the short-circuit operator is fine
             * because it will be deterministic. */
            size_t odd_length = n / 2;
            bool odd_right_heavy = odd_length % 2 == 1
                && (right_heavy || n % 2 == 0);
            merge_slice(data + elem_size * skip, n / 2, elem_size, skip * 2,
                    odd_right_heavy, comparator);

            /* Sort adjacent pairs such that one pair crosses the left-right
             * boundary, which depends on whether the sorted list is
             * right-heavy. If the left sorted half has an even length, then we
             * start at 1; otherweise, we start at 0. We compute this by taking
             * half the total length, and the left half will have an extra
             * member if the total length is odd, and we are not right-heavy.
             * The short-circuit operator is deterministic. Taking this mod 2
             * and then inverting it by subtracting it from 1 gives the
             * starting index. */
            for (size_t i = 1 - (n / 2 + (n % 2 == 1 && !right_heavy)) % 2; i < n - 1;
                    i += 2) {
                sort_swap(data + elem_size * skip * i,
                        data + elem_size * skip * (i + 1), elem_size,
                        comparator);
            }
            break;
         }
    }
}
