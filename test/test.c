#include <stdio.h>
#include "algorithms.h"
#include "opagedmem.h"
#include "oram.h"

int main(void) {
    char *err;
    err = test_sort();
    if (err) {
        printf("Failed o_sort: %s\n", err);
        return 1;
    }
    err = test_sort_generate_swaps();
    if (err) {
        printf("Failed o_sort_generate_swaps: %s\n", err);
        return 1;
    }
    err = test_compact();
    if (err) {
        printf("Failed o_compact: %s\n", err);
        return 1;
    }
    err = test_oram();
    if (err) {
        printf("Failed oram: %s\n", err);
        return 1;
    }
    err = test_opagedmem();
    if (err) {
        printf("Failed opagedmem: %s\n", err);
        return 1;
    }
    return 0;
}
