#include "common.h"
#include <stdlib.h>

uint64_t get_random(void) {
    return ((uint64_t) rand() << 32) | rand();
}
