#include "liboblivious/oram.h"
#include <stdlib.h>
#include <string.h>

int oram_init(oram_t *oram, size_t num_blocks, size_t stash_size) {
    /* Ceiling divide number of blocks by blocks per bucket to get the number
     * of buckets. */
    size_t requested_buckets =
        (num_blocks + ORAM_BLOCKS_PER_BUCKET - 1) / ORAM_BLOCKS_PER_BUCKET;

    /* Round up to the nearest power of 2, minus 1. */
    size_t depth = 1;
    while ((1u << (depth + 1)) - 1 < requested_buckets) {
        depth++;
    }

    /* Allocate buckets. */
    size_t num_buckets = (1u << (depth + 1)) - 1;
    oram->depth = depth;
    oram->buckets = calloc(num_buckets, sizeof(struct oram_bucket));
    if (!oram->buckets) {
        goto exit;
    }

    /* Allocate stash. */
    oram->stash_size = stash_size;
    oram->stash = calloc(stash_size, sizeof(struct oram_block));
    if (!oram->stash) {
        goto exit_free_buckets;
    }

    return 0;

exit_free_buckets:
    free(oram->buckets);
exit:
    return -1;
}

void oram_destroy(oram_t *oram) {
    free(oram->buckets);
    free(oram->stash);
}
