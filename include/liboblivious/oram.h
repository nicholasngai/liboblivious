#ifndef __LIBOBLIVIOUS_ORAM_H
#define __LIBOBLIVIOUS_ORAM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ORAM_BLOCKS_PER_BUCKET 4

struct oram_block {
    bool valid;
    uint64_t id;
    unsigned char data[4096];
};

struct oram_bucket {
    struct oram_block blocks[ORAM_BLOCKS_PER_BUCKET];
};

typedef struct oram {
    size_t depth;
    size_t stash_size;
    struct oram_bucket *buckets;
    struct oram_block *stash;
} oram_t;

int oram_init(oram_t *oram, size_t num_blocks, size_t stash_size);
void oram_destroy(oram_t *oram);

#endif /* liboblivious/oram.h */
