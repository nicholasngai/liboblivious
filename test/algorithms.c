#include "algorithms.h"
#include <stdbool.h>
#include <stdlib.h>
#include "liboblivious/algorithms.h"
#include "common.h"

#define SORT_SIZE 1000

struct comparator_aux {
    bool reverse;
};
static int comparator(const void *a_, const void *b_, void *aux_) {
    const unsigned long *a = a_;
    const unsigned long *b = b_;
    struct comparator_aux *aux = aux_;
    int ret = *a < *b ? -1 : *a > *b ? 1 : 0;
    if (aux->reverse) {
        ret *= -1;
    }
    return ret;
}

char *test_sort(void) {
    char *ret;

    unsigned long *arr = malloc(SORT_SIZE * sizeof(unsigned long));
    if (!arr) {
        ret = "Malloc arr";
        goto exit;
    }

    for (size_t i = 0; i < SORT_SIZE; i++) {
        arr[i] = get_random() % 1000;
    }

    struct comparator_aux aux = {
        .reverse = true,
    };
    o_sort(arr, SORT_SIZE, sizeof(*arr), comparator, &aux);

    bool correct = true;
    for (size_t i = 0; i < SORT_SIZE - 1; i++) {
        if (arr[i] < arr[i + 1]) {
            correct = false;
        }
    }
    if (!correct) {
        ret = "Incorrectly sorted";
        goto exit_free_arr;
    }

    ret = NULL;

exit_free_arr:
    free(arr);
exit:
    return ret;
}

static void swap(size_t a, size_t b, void *arr_) {
    unsigned long *arr = arr_;
    if (arr[a] > arr[b]) {
        unsigned long t = arr[a];
        arr[a] = arr[b];
        arr[b] = t;
    }
}

char *test_sort_generate_swaps(void) {
    char *ret;

    unsigned long *arr = malloc(SORT_SIZE * sizeof(unsigned long));
    if (!arr) {
        ret = "Malloc arr";
        goto exit;
    }

    for (size_t i = 0; i < SORT_SIZE; i++) {
        arr[i] = get_random() % 1000;
    }

    o_sort_generate_swaps(SORT_SIZE, swap, arr);

    bool correct = true;
    for (size_t i = 0; i < SORT_SIZE - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            correct = false;
        }
    }
    if (!correct) {
        ret = "Incorrectly sorted";
        goto exit_free_arr;
    }

    ret = NULL;

exit_free_arr:
    free(arr);
exit:
    return ret;
}

static bool is_marked(const void *elem,  void *aux UNUSED) {
    return *((const bool *) elem);
}

char *test_compact(void) {
    char *ret;

    bool *arr = malloc(SORT_SIZE * sizeof(bool));
    for (size_t i = 0; i < SORT_SIZE; i++) {
        arr[i] = get_random() % 1000 < 200;
    }

    o_compact(arr, SORT_SIZE, sizeof(*arr), is_marked, NULL);

    bool is_marked = true;
    bool correct = true;
    for (size_t i = 0; i < SORT_SIZE; i++) {
        if (is_marked) {
            if (!arr[i]) {
                is_marked = false;
            }
        } else {
            if (arr[i]) {
                correct = false;
                break;
            }
        }
    }
    if (!correct) {
        ret = "Incorrectly compacted";
        goto exit;
    }

    ret = NULL;

exit:
    return ret;
}
