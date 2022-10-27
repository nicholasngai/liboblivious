#include "liboblivious/algorithms.h"
#include "liboblivious/internal/util.h"
#include "liboblivious/primitives.h"

/* Sorting. */

static void sort_swap(void *a, void *b, size_t elem_size,
        int (*comparator)(const void *a, const void *b, void *aux), void *aux) {
    int comp = comparator(a, b, aux);
    o_memswap(a, b, elem_size, comp > 0);
}

static void merge_slice(void *data_, size_t n, size_t elem_size, size_t skip,
        bool right_heavy,
        int (*comparator)(const void *a, const void *b, void *aux), void *aux) {
    unsigned char *data = data_;

    switch (n) {
        case 0:
        case 1:
            /* Do nothing. */
            break;
        case 2: {
            sort_swap(data, data + elem_size * skip, elem_size, comparator,
                    aux);
            break;
        }
        default: {
            /* Even slices are right-heavy iff the even slice has an odd length
             * and the current slice is right-heavy. The short-circuit operator
             * is fine because the whole sort is determinstic. */
            size_t even_length = (n + 1) / 2;
            bool even_right_heavy = even_length % 2 == 1 && right_heavy;
            merge_slice(data, even_length, elem_size, skip * 2,
                    even_right_heavy, comparator, aux);
            /* Odd slices are right-heavy iff the odd slice has an odd length
             * and either the current slice is right-heavy or the current slice
             * has an even length. Again, the short-circuit operator is fine
             * because it will be deterministic. */
            size_t odd_length = n / 2;
            bool odd_right_heavy = odd_length % 2 == 1
                && (right_heavy || n % 2 == 0);
            merge_slice(data + elem_size * skip, odd_length, elem_size,
                    skip * 2, odd_right_heavy, comparator, aux);

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
                sort_swap(data + elem_size * skip * i,
                        data + elem_size * skip * (i + 1), elem_size,
                        comparator, aux);
            }
            break;
         }
    }
}

static void sort_slice(void *data_, size_t n, size_t elem_size, size_t skip,
        int (*comparator)(const void *a, const void *b, void *aux), void *aux) {
    unsigned char *data = data_;

    switch (n) {
        case 0:
        case 1:
            /* Do nothing. */
            break;
        case 2: {
            sort_swap(data, data + elem_size * skip, elem_size, comparator,
                    aux);
            break;
        }
        default: {
            /* Sort left and right halves. Sorting doesn't care if it's
             * right-heavy. */
            size_t left_length = (n + 1) / 2;
            size_t right_length = n / 2;
            sort_slice(data, left_length, elem_size, skip, comparator, aux);
            sort_slice(data + elem_size * skip * left_length, right_length,
                    elem_size, skip, comparator, aux);

            /* Odd-even merge. */
            merge_slice(data, n, elem_size, skip, false, comparator, aux);
            break;
        }
    }
}

void o_sort(void *data, size_t n, size_t elem_size,
        int (*comparator)(const void *a, const void *b, void *aux), void *aux) {
    /* This is an implementation of Batcher's odd-even mergesort, with an
     * additional right-heavy flag to work for arbitrary-sized arrays, not just
     * powers of 2. */
    sort_slice(data, n, elem_size, 1, comparator, aux);
}

/* Sort swap generation. */

static void merge_indices(size_t n, size_t start, size_t skip, bool right_heavy,
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
            /* See merge_slice for explanation. */

            size_t even_length = (n + 1) / 2;
            bool even_right_heavy = even_length % 2 == 1 && right_heavy;
            merge_indices(even_length, start, skip * 2, even_right_heavy, func,
                    aux);

            size_t odd_length = n / 2;
            bool odd_right_heavy = odd_length % 2 == 1
                && (right_heavy || n % 2 == 0);
            merge_indices(odd_length, start + skip, skip * 2, odd_right_heavy,
                    func, aux);

            for (size_t i = 1 - (n / 2 + (n % 2 == 1 && !right_heavy)) % 2;
                    i < n - 1; i += 2) {
                func(start + i * skip, start + (i + 1) * skip, aux);
            }
            break;
         }
    }
}

static void sort_indices(size_t n, size_t start, size_t skip,
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
            /* See sort_slice for explanation. */

            size_t left_length = (n + 1) / 2;
            size_t right_length = n / 2;
            sort_indices(left_length, start, skip, func, aux);
            sort_indices(right_length, start + left_length * skip, skip, func,
                    aux);
            merge_indices(n, start, skip, false, func, aux);
            break;
        }
    }
}

void o_sort_generate_swaps(size_t n,
        void (*func)(size_t a, size_t b, void *aux), void *aux) {
    sort_indices(n, 0, 1, func, aux);
}

/* Compaction. */

static void compact_offset(void *data_, size_t n, size_t elem_size,
        size_t offset, bool (*is_marked)(const void *elem, void *aux),
        void *aux) {
    unsigned char *data = data_;

    if (n < 2) {
        return;
    }

    if (n == 2) {
        bool left_is_marked = is_marked(data, aux);
        bool right_is_marked = is_marked(data + elem_size, aux);
        o_memswap(data, data + elem_size, elem_size,
                (!left_is_marked & right_is_marked) != offset);
        return;
    }

    /* Count the number of marked items in the left half. */
    size_t left_marked_count = 0;
    for (size_t i = 0; i < n / 2; i++) {
        left_marked_count += is_marked(data + elem_size * i, aux);
    }

    /* Compact the left half to an offset of OFFSET % (N / 2). */
    compact_offset(data, n / 2, elem_size, offset % (n / 2), is_marked, aux);

    /* Compact the right half to an offset of
     * (OFFSET + LEFT_MARKED_COUNT) % (N / 2). */
    compact_offset(data, n / 2, elem_size,
            (offset + left_marked_count) % (n / 2), is_marked, aux);

    /* Perform a range of swaps to place the compaction result at the right
     * offset. */
    bool s =
        ((offset % (n / 2)) + left_marked_count >= n / 2) != (offset >= n / 2);
    for (size_t i = 0; i < n / 2; i++) {
        bool cond = s != (i >= (offset + left_marked_count) % (n / 2));
        o_memswap(data + elem_size * i, data + elem_size * (i + n / 2),
                elem_size, cond);
    }
}

void o_compact(void *data_, size_t n, size_t elem_size,
        bool (*is_marked)(const void *elem, void *aux), void *aux) {
    unsigned char *data = data_;

    if (n < 2) {
        return;
    }

    size_t right_length = 1u << ilog2l(n);
    size_t left_length = n - right_length;

    /* Count the number of marked items in the left half. */
    size_t left_marked_count = 0;
    for (size_t i = 0; i < left_length; i++) {
        left_marked_count += is_marked(data + elem_size * i, aux);
    }

    o_compact(data, left_length, elem_size, is_marked, aux);
    compact_offset(data + elem_size * left_length, right_length, elem_size,
            right_length - left_length + left_marked_count, is_marked, aux);

    for (size_t i = 0; i < left_length; i++) {
        bool cond = i > left_marked_count;
        o_memswap(data + elem_size * i, data + elem_size * (i + n / 2),
                elem_size, cond);
    }
}
