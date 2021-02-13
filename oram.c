#include "liboblivious/oram.h"
#include <stdlib.h>
#include <string.h>
#include "liboblivious/primitives.h"

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
        /* Obliviousness violation - out of memory. */
        goto exit;
    }

    /* Allocate stash. */
    oram->stash_size = stash_size;
    oram->stash = calloc(stash_size, sizeof(struct oram_block));
    if (!oram->stash) {
        /* Obliviousness violation - out of memory. */
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

int oram_access(oram_t *oram, uint64_t block_id, uint64_t leaf_id, void *data,
        bool write) {
    if (leaf_id >= 1u << oram->depth) {
        /* Obliviousness violation - invalid leaf ID. */
        goto exit;
    }

    /* Iterate from leaf to root, keeping track of whether the searched block
     * was written and obliviously either writing to or reading from
     * (o_memaccess) the data. We write to the end of the stash, where the
     * dummy blocks should be, stopping one element before the end for a new
     * write if the block was not found. */

    size_t leaf_idx_plus_one = leaf_id + (1u << oram->depth);
    size_t stash_idx = oram->stash_size - 2 - oram->depth;

    if (oram->stash[stash_idx].valid) {
        /* Obliviousness violation - stash overflowed. */
        goto exit;
    }

    bool accessed = false;
    while (leaf_idx_plus_one) {
        memcpy(&oram->stash[stash_idx],
                &oram->buckets->blocks[leaf_idx_plus_one - 1],
                sizeof(struct oram_block));

        bool cond = oram->stash[stash_idx].id == block_id
            && oram->stash[stash_idx].valid;
        accessed = accessed || cond;
        o_memaccess(data, &oram->stash[stash_idx].data, ORAM_BLOCK_SIZE, write,
                cond);

        leaf_idx_plus_one >>= 1;
        stash_idx++;
    }

    /* Write to the final position in the stash iff this is a write and the
     * block was not found, meaning this is a new block ID.  assignments don't
     * need to be conditional because the end of the stash will always be
     * invalid (dummy). */
    bool dummy = write && !accessed;
    oram->stash[oram->stash_size - 1].valid = dummy;
    oram->stash[oram->stash_size - 1].id = block_id;
    memcpy(&oram->stash[oram->stash_size - 1].data, data, ORAM_BLOCK_SIZE);

    // TODO Write back to the path.

    return 0;

exit:
    return -1;
}
