#include "common.h"
#include <stdlib.h>

unsigned long get_random(void) {
    return ((unsigned long) rand() << 32) | rand();
}
