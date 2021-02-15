#include <stdio.h>
#include "oram.h"

int main(void) {
    char *err;
    err = test_oram();
    if (err) {
        printf("Failed: %s\n", err);
        return 1;
    }
}
