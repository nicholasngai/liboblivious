#include "oram.h"
#include <string.h>
#include <stdlib.h>
#include "liboblivious/oram.h"
#include "common.h"

#define ORAM_BLOCK_SIZE 4096
#define ORAM_BLOCKS_PER_BUCKET 4
#define ORAM_NUM_BLOCKS 1024
#define ORAM_STASH_SIZE 256

char *test_oram(void) {
    oram_t oram;
    uint64_t next_leaf_id;
    unsigned char *data;
    char *ret = NULL;

    if (oram_init(&oram, ORAM_BLOCK_SIZE, ORAM_BLOCKS_PER_BUCKET,
                ORAM_NUM_BLOCKS, ORAM_STASH_SIZE)) {
        ret = "Init ORAM";
        goto exit;
    }

    data = malloc(ORAM_BLOCK_SIZE);
    if (!data) {
        ret = "Malloc data1";
        goto exit_destroy_oram;
    }

    /* Invalid read from ORAM. */
    memset(data, '\0', ORAM_BLOCK_SIZE);
    if (!oram_access(&oram, 0x123, 0, data, false, &next_leaf_id, true,
                get_random)) {
        ret = "Invalid read succeeded";
        goto exit_free_data;
    }

    /* Dummy invalid read from ORAM. */
    memset(data, '\0', ORAM_BLOCK_SIZE);
    if (oram_access(&oram, 0x123, 0, data, false, &next_leaf_id, false,
                get_random)) {
        ret = "Dummy invalid valid read failed";
        goto exit_free_data;
    }

    /* Write to ORAM. */
    memset(data, 'A', ORAM_BLOCK_SIZE);
    if (oram_access(&oram, 0x123, 0, data, true, &next_leaf_id, true,
                get_random)) {
        ret = "Write failed";
        goto exit_free_data;
    }

    /* Read from ORAM. */
    memset(data, '\0', ORAM_BLOCK_SIZE);
    if (oram_access(&oram, 0x123, next_leaf_id, data, false, &next_leaf_id,
                true, get_random)) {
        ret = "Read after write failed";
        goto exit_free_data;
    }
    for (size_t i = 0; i < ORAM_BLOCK_SIZE; i++) {
        if (data[i] != 'A') {
            ret = "Read after write produced incorrect data";
            goto exit_free_data;
        }
    }

    /* Dummy write to ORAM. */
    memset(data, 'C', ORAM_BLOCK_SIZE);
    if (oram_access(&oram, 0x123, 0, data, true, &next_leaf_id, false,
                get_random)) {
        ret = "Dummy write";
        goto exit_free_data;
    }

    /* Read from ORAM. */
    memset(data, '\0', ORAM_BLOCK_SIZE);
    if (oram_access(&oram, 0x123, next_leaf_id, data, false, &next_leaf_id,
                true, get_random)) {
        ret = "Read after dummy write failed";
        goto exit_free_data;
    }
    for (size_t i = 0; i < ORAM_BLOCK_SIZE; i++) {
        if (data[i] != 'A') {
            ret = "Read after dummy write produced incorrect data";
            goto exit_free_data;
        }
    }

    /* Write to ORAM. */
    memset(data, 'B', ORAM_BLOCK_SIZE);
    if (oram_access(&oram, 0x123, 0, data, true, &next_leaf_id, true,
                get_random)) {
        ret = "Overwrite";
        goto exit_free_data;
    }

    /* Read from ORAM. */
    memset(data, '\0', ORAM_BLOCK_SIZE);
    if (oram_access(&oram, 0x123, next_leaf_id, data, false, &next_leaf_id,
                true, get_random)) {
        ret = "Read after overwrite failed";
        goto exit_free_data;
    }
    for (size_t i = 0; i < ORAM_BLOCK_SIZE; i++) {
        if (data[i] != 'B') {
            ret = "Read after overwrite produced incorrect data";
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
