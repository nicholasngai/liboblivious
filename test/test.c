#include <stdio.h>
#include "opagetable.h"
#include "oram.h"

int main(void) {
    char *err;
    err = test_oram();
    if (err) {
        printf("Failed oram: %s\n", err);
        return 1;
    }
    err = test_opagetable();
    if (err) {
        printf("Failed opagetable: %s\n", err);
        return 1;
    }
}
