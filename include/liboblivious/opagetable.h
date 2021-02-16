#ifndef __LIBOBLIVIOUS_OPAGETABLE_H
#define __LIBOBLIVIOUS_OPAGETABLE_H

#include <stddef.h>
#include "liboblivious/oram.h"

#define OPAGETABLE_ORAM_BLOCKS_PER_BUCKET 4
#define OPAGETABLE_ORAM_STASH_SIZE 256

/* Page tables use a 12-8-8-8-8-8-12 split to implement a 64-bit address. With
 * 2^4 bytes per PTE, OPAGETABLE_MID_BITS must be exactly 4 less than
 * OPAGETABLE_OFFSET_BITS. */
#define OPAGETABLE_FIRST_BITS 12
#define OPAGETABLE_FIRST_SIZE (1u << OPAGETABLE_FIRST_BITS)
#define OPAGETABLE_FIRST_MASK (OPAGETABLE_FIRST_SIZE - 1)
#define OPAGETABLE_MID_BITS 8
#define OPAGETABLE_MID_SIZE (1u << OPAGETABLE_MID_BITS)
#define OPAGETABLE_MID_MASK (OPAGETABLE_MID_SIZE - 1)
#define OPAGETABLE_MID_COUNT 5
#define OPAGETABLE_OFFSET_BITS 12
#define OPAGETABLE_OFFSET_SIZE (1u << OPAGETABLE_OFFSET_BITS)
#define OPAGETABLE_OFFSET_MASK (OPAGETABLE_OFFSET_SIZE - 1)
#define OPAGETABLE_PAGE_SIZE \
    (OPAGETABLE_MID_SIZE * sizeof(struct opagetable_entry))

struct opagetable_entry {
    bool valid : 1;
    uint64_t block_id : 63;
    uint64_t leaf_id;
};

struct opagetable_table {
    struct opagetable_entry entries[OPAGETABLE_MID_SIZE];
};

typedef struct opagetable {
    oram_t oram;
    struct opagetable_entry *first_level;
    struct opagetable_table *buffer;
    void *data_buffer;
} opagetable_t;

int opagetable_init(opagetable_t *opagetable, size_t num_bytes);
void opagetable_destroy(opagetable_t *opagetable);

int opagetable_access(opagetable_t *opagetable, uint64_t addr, void *data,
        size_t size, bool write, bool *found, uint64_t (*random_func)(void));

#endif /* liboblivious/opagetable.h */
