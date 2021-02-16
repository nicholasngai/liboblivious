#include "algorithms.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "liboblivious/algorithms.h"
#include "common.h"

#define SORT_SIZE 4

static int comparator(void *a, void *b);

char *test_sort(void) {
    char *ret;

    uint64_t *arr = malloc(SORT_SIZE * sizeof(uint64_t));
    if (!arr) {
        ret = "Malloc arr";
        goto exit;
    }

    for (size_t i = 0; i < SORT_SIZE; i++) {
        arr[i] = get_random() % 1000;
    }

    o_sort(arr, SORT_SIZE, sizeof(*arr), comparator);

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

static int comparator(void *a_, void *b_) {
    uint64_t *a = a_;
    uint64_t *b = b_;
    return *a < *b ? -1 : *a > *b ? 1 : 0;
}
