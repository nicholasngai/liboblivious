#ifndef __LIBOBLIVIOUS_ORAM_H
#define __LIBOBLIVIOUS_ORAM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct oram_block {
    bool valid;                 /* Whether this is a valid block. */
    uint64_t id;                /* The block ID. */
    uint64_t leaf_idx_plus_one; /* The block's leaf's index, plus one. */
    unsigned char data[];       /* The data in the block. */
};

struct oram_stash_block {
    uint64_t bucket_idx_plus_one;   /* Temporary - The index of the bucket to
                                       evict this block to, plus one. */
    struct oram_block block;
};

typedef struct oram {
    size_t block_size;
    size_t blocks_per_bucket;
    size_t depth;
    size_t stash_size;
    struct oram_block *buckets;
    struct oram_stash_block *stash;
} oram_t;

int oram_init(oram_t *oram, size_t block_size, size_t blocks_per_bucket,
        size_t num_blocks, size_t stash_size);
void oram_destroy(oram_t *oram);

int oram_access(oram_t *oram, uint64_t block_id, uint64_t leaf_id, void *data,
        bool write, uint64_t *new_leaf_id, uint64_t rand);

#endif /* liboblivious/oram.h */
