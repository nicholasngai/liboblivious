#include "opagedmem.h"
#include "liboblivious/opagedmem.h"
#include "common.h"

char *test_opagedmem(void) {
    char *ret;
    int64_t val;
    bool found;

    opagedmem_t opagedmem;
    if (opagedmem_init(&opagedmem, 1048576)) {
        ret = "Init page table";
        goto exit;
    }

    /* Read from 0xdeadbeefdeadbeef, expect not found. */
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, &found, get_random)) {
        ret = "First read failed";
        goto exit_destroy_opagedmem;
    }
    if (found) {
        ret = "First read found when empty";
        goto exit_destroy_opagedmem;
    }

    /* Write 123 to 0xdeadbeefdeadbeef. */
    val = 123;
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                true, &found, get_random)) {
        ret = "First write failed";
        goto exit_destroy_opagedmem;
    }
    if (found) {
        ret = "First write found when empty";
        goto exit_destroy_opagedmem;
    }

    /* Read from 0xdeadbeefdeadbeef. Expect 123. */
    val = 0;
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, &found, get_random)) {
        ret = "Second read failed";
        goto exit_destroy_opagedmem;
    }
    if (!found) {
        ret = "Second read didn't find";
        goto exit_destroy_opagedmem;
    }
    if (val != 123) {
        ret = "Second read produced incorrect value";
        goto exit_destroy_opagedmem;
    }

    /* Write 456 to 0xdeadbeefcafebabe. */
    val = 456;
    if (opagedmem_access(&opagedmem, 0xdeadbeefcafebabe, &val, sizeof(val),
                true, &found, get_random)) {
        ret = "Second write failed";
        goto exit_destroy_opagedmem;
    }

    /* Read from 0xdeadbeefcafebabe. Expect 456. */
    val = 0;
    if (opagedmem_access(&opagedmem, 0xdeadbeefcafebabe, &val, sizeof(val),
                false, &found, get_random)) {
        ret = "Third read failed";
        goto exit_destroy_opagedmem;
    }
    if (!found) {
        ret = "Third read didn't find";
        goto exit_destroy_opagedmem;
    }
    if (val != 456) {
        ret = "Third read produced incorrect value";
        goto exit_destroy_opagedmem;
    }

    /* Read from 0xdeadbeefdeadbeef. Expect 123. */
    val = 0;
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, &found, get_random)) {
        ret = "Fourth read failed";
        goto exit_destroy_opagedmem;
    }
    if (!found) {
        ret = "Fourth read didn't find";
        goto exit_destroy_opagedmem;
    }
    if (val != 123) {
        ret = "Fourth read produced incorrect value";
        goto exit_destroy_opagedmem;
    }

    ret = NULL;

exit_destroy_opagedmem:
    opagedmem_destroy(&opagedmem);
exit:
    return ret;
}
