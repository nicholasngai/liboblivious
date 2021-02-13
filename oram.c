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
    size_t stash_idx =
        oram->stash_size - 2 - oram->depth * ORAM_BLOCKS_PER_BUCKET;

    if (oram->stash[stash_idx].valid) {
        /* Obliviousness violation - stash overflowed. */
        goto exit;
    }
    while (leaf_idx_plus_one) {
        /* Copy block to stash. */
        memcpy(&oram->stash[stash_idx],
                &oram->buckets[leaf_idx_plus_one - 1].blocks,
                sizeof(struct oram_block));

        for (size_t i = 0; i < ORAM_BLOCKS_PER_BUCKET; i++) {
            /* Invalidate block that we copied. */
            oram->buckets[leaf_idx_plus_one - 1].blocks[i].valid = false;
            /* Track the leaf ID in the stash. */
            oram->stash[stash_idx + i].leaf_id = leaf_id;
        }

        leaf_idx_plus_one >>= 1;
        stash_idx += ORAM_BLOCKS_PER_BUCKET;
    }

    /* Obliviously scan through the stash and access the block. The last
     * position should be a dummy if we have reached this point, so we skip it
     * and access it after this loop.  */
    bool accessed = false;
    for (size_t i = 0; i < oram->stash_size - 1; i++) {
        bool cond = oram->stash[i].id == block_id && oram->stash[i].valid;
        accessed = accessed || cond;
        o_memaccess(data, &oram->stash[i].data, ORAM_BLOCK_SIZE, write, cond);
    }

    /* Write to the final position in the stash iff this is a write and the
     * desired block was not accessed, meaning this is a new block ID.
     * Assignments don't need to be conditional because the end of the stash
     * will always be invalid (dummy). */
    bool dummy = write && !accessed;
    oram->stash[oram->stash_size - 1].valid = dummy;
    oram->stash[oram->stash_size - 1].id = block_id;
    memcpy(&oram->stash[oram->stash_size - 1].data, data, ORAM_BLOCK_SIZE);

    // TODO Write back to the path.

    return 0;

exit:
    return -1;
}
