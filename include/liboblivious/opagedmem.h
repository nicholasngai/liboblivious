#ifndef LIBOBLIVIOUS_OPAGEDMEM_H
#define LIBOBLIVIOUS_OPAGEDMEM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "liboblivious/internal/defs.h"
#include "liboblivious/oram.h"

#define OPAGEDMEM_ORAM_BLOCKS_PER_BUCKET 4
#define OPAGEDMEM_ORAM_STASH_SIZE 160

/* Page tables use a 14-5-5-5-5-5-5-5-5-10 split to implement a 64-bit address.
 * With 16 bytes per PTE, OPAGEDMEM_MID_BITS must be exactly 4 less than
 * OPAGEDMEM_OFFSET_BITS. */
#define OPAGEDMEM_FIRST_BITS 12
#define OPAGEDMEM_FIRST_SIZE (1u << OPAGEDMEM_FIRST_BITS)
#define OPAGEDMEM_FIRST_MASK (OPAGEDMEM_FIRST_SIZE - 1)
#define OPAGEDMEM_MID_BITS 6
#define OPAGEDMEM_MID_SIZE (1u << OPAGEDMEM_MID_BITS)
#define OPAGEDMEM_MID_MASK (OPAGEDMEM_MID_SIZE - 1)
#define OPAGEDMEM_MID_COUNT 7
#define OPAGEDMEM_OFFSET_BITS 10
#define OPAGEDMEM_PAGE_SIZE \
    (OPAGEDMEM_MID_SIZE * sizeof(struct opagedmem_entry))
#define OPAGEDMEM_OFFSET_MASK (OPAGEDMEM_PAGE_SIZE - 1)

LIBOBLIVIOUS_EXTERNC_BEGIN

struct opagedmem_entry {
    uint64_t leaf_id;
    bool valid;
    unsigned char unused[7];
};

struct opagedmem_table {
    struct opagedmem_entry entries[OPAGEDMEM_MID_SIZE];
};

typedef struct opagedmem {
    oram_t oram;
    struct opagedmem_entry *first_level;
    struct opagedmem_table *buffer;
    void *data_buffer;
} opagedmem_t;

int opagedmem_init(opagedmem_t *opagedmem, size_t num_bytes);
void opagedmem_destroy(opagedmem_t *opagedmem);

/* Performs an oblivious access within a single page of the oblivious paged
 * memory. Accesses that span multiple pages are rejected non-obliviously. */
int opagedmem_pageaccess(opagedmem_t *opagedmem, uint64_t addr, void *data,
        size_t size, bool write, bool is_real_access,
        uint64_t (*random_func)(void));

/* Performs an oblivious access of the oblivious paged memory.
 *
 * Unlike opagedmem_pageaccess, the access need not be within a single page, at
 * the expense of slower access times. Also unlike opagedmem_pageaccess, the
 * size of the access is only oblivious up the level of the largest potential
 * number of spanned pages (i.e. the value of SIZE / OPAGEDMEM_OFFSET_SIZE) is */
int opagedmem_access(opagedmem_t *opagedmem, uint64_t addr, void *data,
        size_t size, bool write, bool is_real_access,
        uint64_t (*random_func)(void));

LIBOBLIVIOUS_EXTERNC_END

#endif /* liboblivious/opagedmem.h */
