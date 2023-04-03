#include "liboblivious/oram.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "liboblivious/algorithms.h"
#include "liboblivious/primitives.h"

/* Helper function for the size of an ORAM block. */
static size_t get_block_size(oram_t *oram) {
    return offsetof(struct oram_block, data) + oram->block_size;
}

/* Helper function for the size of an ORAM stash block. */
static size_t get_stash_block_size(oram_t *oram) {
    return offsetof(struct oram_stash_block, block) + get_block_size(oram);
}

/* Helper function to return a pointer to a block in a bucket. */
static struct oram_block *get_bucket_block(oram_t *oram, size_t bucket_idx,
        size_t block_idx) {
    return (struct oram_block *) ((unsigned char *) oram->buckets
            + (bucket_idx * oram->blocks_per_bucket + block_idx)
                * get_block_size(oram));
}

/* Helper function to return a pointer to a block in the stash. */
static struct oram_stash_block *get_stash_block(oram_t *oram,
        size_t stash_idx) {
    return (struct oram_stash_block *) ((unsigned char *) oram->stash
            + stash_idx * get_stash_block_size(oram));
}

int oram_init(oram_t *oram, size_t block_size, size_t blocks_per_bucket,
        size_t num_blocks, size_t stash_size) {
    oram->block_size = block_size;
    oram->blocks_per_bucket = blocks_per_bucket;

    /* Ceiling divide number of blocks by blocks per bucket to get the number
     * of buckets. */
    size_t requested_buckets =
        (num_blocks + oram->blocks_per_bucket - 1) / oram->blocks_per_bucket;

    /* Round up to the nearest power of 2, minus 1. */
    size_t depth = 1;
    while ((1u << depth) - 1 < requested_buckets) {
        depth++;
    }

    /* Allocate buckets. */
    size_t num_buckets = (1u << depth) - 1;
    oram->depth = depth;
    oram->buckets = calloc(num_buckets,
            blocks_per_bucket * get_block_size(oram));
    if (!oram->buckets) {
        /* Obliviousness violation - out of memory. */
        goto exit;
    }

    /* Allocate stash. */
    oram->stash_size = stash_size;
    oram->stash = calloc(stash_size, get_stash_block_size(oram));
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

/* Comparator to sort blocks from highest to lowest bucket index. */
static int stash_comparator(const void *a_, const void *b_, void *aux UNUSED) {
    const struct oram_stash_block *a = a_;
    const struct oram_stash_block *b = b_;

    /* If A < B, this adds 1 + (1 - 1) == 1.
     * If A > B, this adds 0 + (0 - 1) == -1.
     * If A == B, this adds 0 + (1 - 1) == 0. */
    int comp = (int) (a->bucket_idx_plus_one < b->bucket_idx_plus_one)
        + ((int) (a->bucket_idx_plus_one <= b->bucket_idx_plus_one) - 1);

    comp <<= 2;

    /* If A is dummy and B is not, this adds 1. */

    comp += !a->block.valid & b->block.valid;

    return comp;
}

int oram_access(oram_t *oram, uint64_t block_id, uint64_t leaf_id, void *data,
        bool write, uint64_t *new_leaf_id, bool is_real_access,
        uint64_t (*rand_func)(void)) {
    size_t bucket_fullness[oram->depth];

    /* If this is a dummy access, choose a random leaf ID. */
    o_set64(&leaf_id, rand_func() % (1u << (oram->depth - 1)), !is_real_access);

    if (leaf_id >= 1u << (oram->depth - 1)) {
        /* Obliviousness violation - invalid leaf ID. */
        goto exit;
    }

    /* We will eventually need to pad the last
     * oram->depth * oram->blocks_per_bucket blocks with dummy blocks, and we
     * also need 1 additional slot for a potential new write if the block isn't
     * found, so we ensure that there is at least that much space at the end of
     * the stash. All the dummy blocks other than the initial path are at the
     * end, so it is sufficient to just check one element. */

    if (get_stash_block(oram, oram->stash_size - oram->depth * oram->blocks_per_bucket - 1)->block.valid) {
        /* Obliviousness violation - stash overflowed. */
        goto exit;
    }

    /* Iterate from leaf to root, keeping track of whether the searched block
     * was written and obliviously either writing to or reading from
     * (o_memaccess) the data. We read the blocks into the beginning of the
     * stash, since the first oram->depth * oram->blocks_per_bucket positions
     * should contain the path of stash blocks that was evicted last time and
     * are thus invalid. */

    size_t leaf_idx_plus_one = leaf_id + (1u << (oram->depth - 1));
    size_t stash_idx = 0;

    for (size_t bucket_idx_plus_one = leaf_idx_plus_one; bucket_idx_plus_one;
            bucket_idx_plus_one >>= 1) {
        for (size_t i = 0; i < oram->blocks_per_bucket; i++) {
            /* Copy block to stash. */
            memcpy(&get_stash_block(oram, stash_idx)->block,
                    get_bucket_block(oram, bucket_idx_plus_one - 1, i),
                    get_block_size(oram));
            /* Invalidate it in the tree. */
            get_bucket_block(oram, bucket_idx_plus_one - 1, i)->valid = false;
            stash_idx++;
        }
    }

    /* Obliviously scan through the stash and access the block. The last
     * (oram->depth * oram->blocks_per_bucket - 1) positions should be a
     * dummies if we have reached this point, so we skip it. The accessed
     * block gets assigned the new leaf. If this is a dummy access, start with
     * accessed == true to begin with and don't change anything. */
    o_set64(new_leaf_id, rand_func() % (1u << (oram->depth - 1)),
                is_real_access);
    size_t new_leaf_idx_plus_one = *new_leaf_id + (1u << (oram->depth - 1));
    bool accessed = false;
    for (size_t i = 0;
            i < oram->stash_size - oram->depth * oram->blocks_per_bucket - 1;
            i++) {
        /* Access the block and set its new leaf if it was requested. */
        bool cond = (get_stash_block(oram, i)->block.id == block_id)
            & get_stash_block(oram, i)->block.valid & is_real_access;
        o_set64(&get_stash_block(oram, i)->block.leaf_idx_plus_one,
                new_leaf_idx_plus_one, cond);
        o_memaccess(data, get_stash_block(oram, i)->block.data,
                oram->block_size, write, cond);
        accessed |= cond;
    }

    stash_idx = oram->stash_size - oram->depth * oram->blocks_per_bucket - 1;

    /* Write to the next position in the stash iff this is a write, the desired
     * block was not accessed, and this is a real access, meaning this is a new
     * block ID. Assignments don't need to be conditional because the end of
     * the stash will always be invalid (dummy). */
    bool cond = write & !accessed & is_real_access;
    get_stash_block(oram, stash_idx)->block.valid = cond;
    get_stash_block(oram, stash_idx)->block.id = block_id;
    get_stash_block(oram, stash_idx)->block.leaf_idx_plus_one =
        new_leaf_idx_plus_one;
    memcpy(get_stash_block(oram, stash_idx)->block.data, data,
            oram->block_size);
    accessed |= cond;
    stash_idx++;

    /* Assign all blocks in the stash to the deepest bucket index possible.
     * Dummy blocks get the max bucket value to get sorted to the end. We stop
     * before the last (oram->depth * oram->blocks_per_bucket) positions, for
     * dummy padding. */
    memset(bucket_fullness, '\0', sizeof(bucket_fullness));
    for (size_t i = 0;
            i < oram->stash_size - oram->depth * oram->blocks_per_bucket; i++) {
        bool assigned = !get_stash_block(oram, i)->block.valid;
        get_stash_block(oram, i)->bucket_idx_plus_one = 0;
        uint64_t curr_idx_plus_one =
            get_stash_block(oram, i)->block.leaf_idx_plus_one;
        uint64_t bucket_idx_plus_one = leaf_idx_plus_one;
        size_t bufu_idx = 0;
        while (bucket_idx_plus_one) {
            bool cond = !assigned & (bucket_idx_plus_one == curr_idx_plus_one)
                & (bucket_fullness[bufu_idx] < oram->blocks_per_bucket);
            o_set64(&get_stash_block(oram, i)->bucket_idx_plus_one,
                    bucket_idx_plus_one, cond);
            o_setsize(&bucket_fullness[bufu_idx], bucket_fullness[bufu_idx] + 1,
                    cond);
            assigned |= cond;
            curr_idx_plus_one >>= 1;
            bucket_idx_plus_one >>= 1;
            bufu_idx++;
        }
    }

    /* Pad the stash with (oram->depth * oram->blocks_per_bucket) dummy blocks,
     * with an assigned bucket index if needed and an invalid value if not,
     * based on the bucket fullness. */
    size_t bufu_idx = 0;
    for (size_t bucket_idx_plus_one = leaf_idx_plus_one; bucket_idx_plus_one;
            bucket_idx_plus_one >>= 1) {
        for (size_t i = 0; i < oram->blocks_per_bucket; i++) {
            bool cond = bucket_fullness[bufu_idx] + i < oram->blocks_per_bucket;
            get_stash_block(oram, stash_idx)->bucket_idx_plus_one = 0;
            o_set64(&get_stash_block(oram, stash_idx)->bucket_idx_plus_one,
                    bucket_idx_plus_one, cond);
            stash_idx++;
        }
        bufu_idx++;
    }

    /* Sort blocks from highest to lowest bucket index, with 0 (invalid) at the
     * end. At this point, each valid index has exactly oram->blocks_per_bucket
     * blocks. */
    o_sort(oram->stash, oram->stash_size, get_stash_block_size(oram),
            stash_comparator, NULL);

    /* The first oram->depth * oram->blocks_per_bucket of the stash now
     * contains the blocks to evict back to the path, so we write them back,
     * invalidating them in the stash. */
    stash_idx = 0;
    for (size_t bucket_idx_plus_one = leaf_idx_plus_one; bucket_idx_plus_one;
            bucket_idx_plus_one >>= 1) {
        for (size_t i = 0; i < oram->blocks_per_bucket; i++) {
            /* Copy block to path. */
            memcpy(get_bucket_block(oram, bucket_idx_plus_one - 1, i),
                    &get_stash_block(oram, stash_idx)->block,
                    get_block_size(oram));
            /* Invalidate it in the stash. */
            get_stash_block(oram, stash_idx)->block.valid = false;
            stash_idx++;
        }
    }

    if (!accessed & is_real_access) {
        goto exit;
    }

    return 0;

exit:
    return -1;
}
