#include "liboblivious/oram.h"
#include <stdlib.h>
#include <string.h>
#include "liboblivious/algorithms.h"
#include "liboblivious/primitives.h"

static int stash_comparator(void *a, void *b);

int oram_init(oram_t *oram, size_t num_blocks, size_t stash_size) {
    /* Ceiling divide number of blocks by blocks per bucket to get the number
     * of buckets. */
    size_t requested_buckets =
        (num_blocks + ORAM_BLOCKS_PER_BUCKET - 1) / ORAM_BLOCKS_PER_BUCKET;

    /* Round up to the nearest power of 2, minus 1. */
    size_t depth = 1;
    while ((1u << depth) - 1 < requested_buckets) {
        depth++;
    }

    /* Allocate buckets. */
    size_t num_buckets = (1u << depth) - 1;
    oram->depth = depth;
    oram->buckets = calloc(num_buckets, sizeof(struct oram_bucket));
    if (!oram->buckets) {
        /* Obliviousness violation - out of memory. */
        goto exit;
    }

    /* Allocate stash. */
    oram->stash_size = stash_size;
    oram->stash = calloc(stash_size, sizeof(struct oram_stash_block));
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
        bool write, uint64_t *new_leaf_id, uint64_t rand) {
    size_t bucket_fullness[oram->depth];

    if (leaf_id >= 1u << (oram->depth - 1)) {
        /* Obliviousness violation - invalid leaf ID. */
        goto exit;
    }

    /* Iterate from leaf to root, keeping track of whether the searched block
     * was written and obliviously either writing to or reading from
     * (o_memaccess) the data. We write to the end of the stash, where the
     * dummy blocks should be, leaving enough space for a full path of dummy
     * blocks plus one element before the end for padding the stash for the
     * writeback proecdure and for a new write if the block was not found. */

    size_t leaf_idx_plus_one = leaf_id + (1u << (oram->depth - 1));
    size_t stash_idx =
        oram->stash_size - 1 - oram->depth * ORAM_BLOCKS_PER_BUCKET * 2;

    if (oram->stash[stash_idx].block.valid) {
        /* Obliviousness violation - stash overflowed. */
        goto exit;
    }

    for (size_t bucket_idx_plus_one = leaf_idx_plus_one; bucket_idx_plus_one;
            bucket_idx_plus_one >>= 1) {
        for (size_t i = 0; i < ORAM_BLOCKS_PER_BUCKET; i++) {
            /* Copy block to stash. */
            memcpy(&oram->stash[stash_idx].block,
                    &oram->buckets[bucket_idx_plus_one - 1].blocks[i],
                    sizeof(struct oram_block));
            /* Invalidate it in the tree. */
            oram->buckets[bucket_idx_plus_one - 1].blocks[i].valid = false;
            stash_idx++;
        }
    }

    /* Obliviously scan through the stash and access the block. The last
     * (oram->depth * ORAM_BLOCKS_PER_BUCKET - 1) positions should be a dummies
     * if we have reached this point, so we skip it.  The accessed block gets
     * assigned the new leaf. */
    *new_leaf_id = rand % (1u << (oram->depth - 1));
    size_t new_leaf_idx_plus_one = *new_leaf_id + (1u << (oram->depth - 1));
    bool accessed = false;
    for (size_t i = 0;
            i < oram->stash_size - oram->depth * ORAM_BLOCKS_PER_BUCKET - 1;
            i++) {
        /* Access the block and set its new leaf if it was requested. */
        bool cond =
            (oram->stash[i].block.id == block_id) & oram->stash[i].block.valid;
        o_set64(&oram->stash[i].block.leaf_idx_plus_one, new_leaf_idx_plus_one,
                cond);
        o_memaccess(data, &oram->stash[i].block.data, ORAM_BLOCK_SIZE, write,
                cond);
        accessed |= cond;
    }

    /* Write to the next position in the stash iff this is a write and the
     * desired block was not accessed, meaning this is a new block ID.
     * Assignments don't need to be conditional because the end of the stash
     * will always be invalid (dummy). */
    bool cond = write & !accessed;
    oram->stash[stash_idx].block.valid = cond;
    oram->stash[stash_idx].block.id = block_id;
    oram->stash[stash_idx].block.leaf_idx_plus_one = new_leaf_idx_plus_one;
    memcpy(&oram->stash[stash_idx].block.data, data, ORAM_BLOCK_SIZE);
    accessed |= cond;
    stash_idx++;

    /* Assign all blocks in the stash to the deepest bucket index possible.
     * Dummy blocks get the max bucket value to get sorted to the end. We stop
     * before the last (oram->depth * ORAM_BLOCKS_PER_BUCKET) positions, for
     * dummy padding. */
    memset(bucket_fullness, '\0', sizeof(bucket_fullness));
    for (size_t i = 0;
            i < oram->stash_size - oram->depth * ORAM_BLOCKS_PER_BUCKET; i++) {
        bool assigned = !oram->stash[i].block.valid;
        oram->stash[i].bucket_idx_plus_one = 0;
        uint64_t curr_idx_plus_one = oram->stash[i].block.leaf_idx_plus_one;
        uint64_t bucket_idx_plus_one = leaf_idx_plus_one;
        size_t bufu_idx = 0;
        while (bucket_idx_plus_one) {
            bool cond = !assigned & (bucket_idx_plus_one == curr_idx_plus_one)
                & (bucket_fullness[bufu_idx] < ORAM_BLOCKS_PER_BUCKET);
            o_set64(&oram->stash[i].bucket_idx_plus_one, bucket_idx_plus_one,
                    cond);
            o_set64(&bucket_fullness[bufu_idx], bucket_fullness[bufu_idx] + 1,
                    cond);
            curr_idx_plus_one >>= 1;
            bucket_idx_plus_one >>= 1;
            bufu_idx++;
        }
    }

    /* Pad the stash with (oram->depth * ORAM_BLOCKS_PER_BUCKET) dummy blocks,
     * with an assigned bucket index if needed and an invalid value if not,
     * based on the bucket fullness. */
    size_t bufu_idx = 0;
    for (size_t bucket_idx_plus_one = leaf_idx_plus_one; bucket_idx_plus_one;
            bucket_idx_plus_one >>= 1) {
        for (size_t i = 0; i < ORAM_BLOCKS_PER_BUCKET; i++) {
            bool cond = bucket_fullness[bufu_idx] + i < ORAM_BLOCKS_PER_BUCKET;
            oram->stash[stash_idx].bucket_idx_plus_one = 0;
            o_set64(&oram->stash[stash_idx].bucket_idx_plus_one,
                    bucket_idx_plus_one, cond);
            stash_idx++;
        }
        bufu_idx++;
    }

    /* Sort blocks from highest to lowest bucket index, with 0 (invalid) at the
     * end. At this point, each valid index has exactly ORAM_BLOCKS_PER_BUCKET
     * blocks. */
    o_sort(oram->stash, oram->stash_size, sizeof(struct oram_stash_block),
            stash_comparator);

    /* The first oram->depth * ORAM_BLOCKS_PER_BUCKET of the stash now contains
     * the blocks to evict back to the path, so we write them back,
     * invalidating them in the stash. */
    stash_idx = 0;
    for (size_t bucket_idx_plus_one = leaf_idx_plus_one; bucket_idx_plus_one;
            bucket_idx_plus_one >>= 1) {
        for (size_t i = 0; i < ORAM_BLOCKS_PER_BUCKET; i++) {
            /* Copy block to stash. */
            memcpy(&oram->buckets[bucket_idx_plus_one - 1].blocks[i],
                    &oram->stash[stash_idx].block,
                    sizeof(struct oram_block));
            /* Invalidate it in the stash. */
            oram->stash[stash_idx].block.valid = false;
            stash_idx++;
        }
    }

    return !accessed;

exit:
    return -1;
}

/* Comparator to sort blocks from highest to lowest bucket index. */
static int stash_comparator(void *a_, void *b_) {
    struct oram_stash_block *a = a_;
    struct oram_stash_block *b = b_;

    /* If A < B, this adds 1 + (1 - 1) == 1.
     * If A > B, this adds 0 + (0 - 1) == -1.
     * If A == B, this adds 0 + (1 - 1) == 0. */
    return (int) (a->bucket_idx_plus_one < b->bucket_idx_plus_one)
        + ((int) (a->bucket_idx_plus_one <= b->bucket_idx_plus_one) - 1);
}
