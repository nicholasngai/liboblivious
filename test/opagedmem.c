#include "opagedmem.h"
#include "liboblivious/opagedmem.h"
#include "common.h"

char *test_opagedmem(void) {
    char *ret;
    int64_t val;

    opagedmem_t opagedmem;
    if (opagedmem_init(&opagedmem, 1048576)) {
        ret = "Init page table";
        goto exit;
    }

    /* Read from 0xdeadbeefdeadbeef, expect 0. */
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, true, get_random)) {
        ret = "Empty read failed";
        goto exit_destroy_opagedmem;
    }
    if (val != 0) {
        ret = "Empty read produced non-zero value";
        goto exit_destroy_opagedmem;
    }

    /* Dummy write to 0xdeadbeefdeadbeef. */
    val = 123;
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                true, false, get_random)) {
        ret = "Empty dummy write failed";
        goto exit_destroy_opagedmem;
    }
    if (val != 123) {
        ret = "Empty dummy write changed val";
        goto exit_destroy_opagedmem;
    }

    /* Read from 0xdeadbeefdeadbeef, expect 0. */
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, true, get_random)) {
        ret = "Empty read after dummy write failed";
        goto exit_destroy_opagedmem;
    }
    if (val != 0) {
        ret = "Empty read after dummy write produced non-zero value";
        goto exit_destroy_opagedmem;
    }

    /* Write 123 to 0xdeadbeefdeadbeef. */
    val = 123;
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                true, true, get_random)) {
        ret = "Empty write failed";
        goto exit_destroy_opagedmem;
    }
    if (val != 123) {
        ret = "Empty write changed val";
        goto exit_destroy_opagedmem;
    }

    /* Read from 0xdeadbeefdeadbeef. Expect 123. */
    val = 0;
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, true, get_random)) {
        ret = "Present read failed";
        goto exit_destroy_opagedmem;
    }
    if (val != 123) {
        ret = "Present read produced incorrect value";
        goto exit_destroy_opagedmem;
    }

    /* Write 456 to 0xdeadbeefcafebabe. */
    val = 456;
    if (opagedmem_access(&opagedmem, 0xdeadbeefcafebabe, &val, sizeof(val),
                true, true, get_random)) {
        ret = "Second empty write failed";
        goto exit_destroy_opagedmem;
    }
    if (val != 456) {
        ret = "Second empty write changd val";
        goto exit_destroy_opagedmem;
    }

    /* Read from 0xdeadbeefcafebabe. Expect 456. */
    val = 0;
    if (opagedmem_access(&opagedmem, 0xdeadbeefcafebabe, &val, sizeof(val),
                false, true, get_random)) {
        ret = "Second read after write failed";
        goto exit_destroy_opagedmem;
    }
    if (val != 456) {
        ret = "Second read after write produced incorrect value";
        goto exit_destroy_opagedmem;
    }

    /* Read from 0xdeadbeefdeadbeef. Expect 123. */
    val = 0;
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, true, get_random)) {
        ret = "Read after second write failed";
        goto exit_destroy_opagedmem;
    }
    if (val != 123) {
        ret = "Read after second write produced incorrect value";
        goto exit_destroy_opagedmem;
    }

    /* Dummy write to 0xdeadbeefdeadbeef. */
    val = 789;
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                true, false, get_random)) {
        ret = "Dummy overwrite failed";
        goto exit_destroy_opagedmem;
    }
    if (val != 789) {
        ret = "Dummy overwrite changed val";
        goto exit_destroy_opagedmem;
    }

    /* Read from 0xdeadbeefdeadbeef, expect 123. */
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, true, get_random)) {
        ret = "Empty read after dummy overwrite failed";
        goto exit_destroy_opagedmem;
    }
    if (val != 123) {
        ret = "Present read after dummy overwrite produced incorrect value";
        goto exit_destroy_opagedmem;
    }

    /* Write 789 to 0xdeadbeefdeadbeef. */
    val = 789;
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                true, true, get_random)) {
        ret = "Overwrite write failed";
        goto exit_destroy_opagedmem;
    }

    /* Read from 0xdeadbeefdeadbeef. Expect 789. */
    val = 0;
    if (opagedmem_access(&opagedmem, 0xdeadbeefdeadbeef, &val, sizeof(val),
                false, true, get_random)) {
        ret = "Read after overwrite failed";
        goto exit_destroy_opagedmem;
    }
    if (val != 789) {
        ret = "Read after overwrite produced incorrect value";
        goto exit_destroy_opagedmem;
    }

    ret = NULL;

exit_destroy_opagedmem:
    opagedmem_destroy(&opagedmem);
exit:
    return ret;
}
