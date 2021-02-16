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

    /* Read from 0xdeadbeefdeadbeef, expect not found. */
    if (opagetable_access(&opagetable, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, &found, get_random)) {
        ret = "First read failed";
        goto exit_destroy_opagetable;
    }
    if (found) {
        ret = "First read found when empty";
        goto exit_destroy_opagetable;
    }

    /* Write 123 to 0xdeadbeefdeadbeef. */
    val = 123;
    if (opagetable_access(&opagetable, 0xdeadbeefdeadbeef, &val, sizeof(val),
                true, &found, get_random)) {
        ret = "First write failed";
        goto exit_destroy_opagetable;
    }
    if (found) {
        ret = "First write found when empty";
        goto exit_destroy_opagetable;
    }

    /* Read from 0xdeadbeefdeadbeef. Expect 123. */
    val = 0;
    if (opagetable_access(&opagetable, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, &found, get_random)) {
        ret = "Second read failed";
        goto exit_destroy_opagetable;
    }
    if (!found) {
        ret = "Second read didn't find";
        goto exit_destroy_opagetable;
    }
    if (val != 123) {
        ret = "Second read produced incorrect value";
        goto exit_destroy_opagetable;
    }

    /* Write 456 to 0xdeadbeefcafebabe. */
    val = 456;
    if (opagetable_access(&opagetable, 0xdeadbeefcafebabe, &val, sizeof(val),
                true, &found, get_random)) {
        ret = "Second write failed";
        goto exit_destroy_opagetable;
    }

    /* Read from 0xdeadbeefcafebabe. Expect 456. */
    val = 0;
    if (opagetable_access(&opagetable, 0xdeadbeefcafebabe, &val, sizeof(val),
                false, &found, get_random)) {
        ret = "Third read failed";
        goto exit_destroy_opagetable;
    }
    if (!found) {
        ret = "Third read didn't find";
        goto exit_destroy_opagetable;
    }
    if (val != 456) {
        ret = "Third read produced incorrect value";
        goto exit_destroy_opagetable;
    }

    /* Read from 0xdeadbeefdeadbeef. Expect 123. */
    val = 0;
    if (opagetable_access(&opagetable, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, &found, get_random)) {
        ret = "Fourth read failed";
        goto exit_destroy_opagetable;
    }
    if (!found) {
        ret = "Fourth read didn't find";
        goto exit_destroy_opagetable;
    }
    if (val != 123) {
        ret = "Fourth read produced incorrect value";
        goto exit_destroy_opagetable;
    }

    ret = NULL;

exit_destroy_opagetable:
    opagetable_destroy(&opagetable);
exit:
    return ret;
}
