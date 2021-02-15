#include "liboblivious/oram.h"
#include <string.h>
#include <stdlib.h>

char *test_oram(void) {
    oram_t oram;
    uint64_t next_leaf_id;
    unsigned char *data;
    char *ret = NULL;

    if (oram_init(&oram, 1024, 256)) {
        ret = "Init ORAM";
        goto exit;
    }

    data = malloc(ORAM_BLOCK_SIZE);
    if (!data) {
        ret = "Malloc data1";
        goto exit_destroy_oram;
    }

    memset(data, 'A', ORAM_BLOCK_SIZE);
    if (oram_access(&oram, 0x123, 0, data, true, &next_leaf_id, random())) {
        ret = "First write";
        goto exit_free_data;
    }
    memset(data, '\0', ORAM_BLOCK_SIZE);
    if (oram_access(&oram, 0x123, next_leaf_id, data, false, &next_leaf_id,
                random())) {
        ret = "First read";
        goto exit_free_data;
    }
    for (size_t i = 0; i < ORAM_BLOCK_SIZE; i++) {
        if (data[i] != 'A') {
            ret = "First read incorrect data";
            goto exit_free_data;
        }
    }

    memset(data, 'B', ORAM_BLOCK_SIZE);
    if (oram_access(&oram, 0x123, 0, data, true, &next_leaf_id, random())) {
        ret = "Second write";
        goto exit_free_data;
    }
    memset(data, '\0', ORAM_BLOCK_SIZE);
    if (oram_access(&oram, 0x123, next_leaf_id, data, false, &next_leaf_id,
                random())) {
        ret = "Second read";
        goto exit_free_data;
    }
    for (size_t i = 0; i < ORAM_BLOCK_SIZE; i++) {
        if (data[i] != 'B') {
            ret = "Second read incorrect data";
            goto exit_free_data;
        }
    }

    ret = NULL;

exit_free_data:
    free(data);
exit_destroy_oram:
    oram_destroy(&oram);
exit:
    return ret;
}
