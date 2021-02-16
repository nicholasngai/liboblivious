#include "opagetable.h"
#include "liboblivious/opagetable.h"
#include "common.h"

char *test_opagetable(void) {
    char *ret;
    int64_t val;
    bool found;

    opagetable_t opagetable;
    if (opagetable_init(&opagetable, 1048576)) {
        ret = "Init page table";
        goto exit;
    }

    ret = NULL;

    if (opagetable_access(&opagetable, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, &found, get_random)) {
        ret = "First access failed";
        goto exit_destroy_opagetable;
    }
    if (found) {
        ret = "First access found when empty";
        goto exit_destroy_opagetable;
    }

exit_destroy_opagetable:
    opagetable_destroy(&opagetable);
exit:
    return ret;
}
