#include "liboblivious/opagedmem.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "liboblivious/internal/util.h"
#include "liboblivious/oram.h"
#include "liboblivious/primitives.h"

static_assert((1 << OPAGEDMEM_OFFSET_BITS) == (1 << OPAGEDMEM_MID_BITS) * sizeof(struct opagedmem_entry),
        "1 << OPAGEDMEM_OFFSET_BITS must be equal to (1 << OPAGEDMEM_MID_BITS) * sizeof(struct opagedmem_entry)");

int opagedmem_init(opagedmem_t *opagedmem, size_t num_bytes) {
    size_t num_blocks = (num_bytes + OPAGEDMEM_PAGE_SIZE - 1)
        / OPAGEDMEM_PAGE_SIZE;

    /* Use OPAGEDMEM_ORAM_STASH_SIZE as the persistent stash size (which
     * doesn't depend on num_bytes) + log2(num_bytes / OPAGEDMEM_PAGE_SIZE) *
     * OPAGEDMEM_ORAM_BLOCKS_PER_BUCKET the transient stash size (which
     * depends on Z * log2(N)). */
    size_t oram_stash_size = 0;
    while ((1u << oram_stash_size) < num_blocks) {
        oram_stash_size++;
    }
    oram_stash_size =
        OPAGEDMEM_ORAM_STASH_SIZE
            + oram_stash_size * OPAGEDMEM_ORAM_BLOCKS_PER_BUCKET;

    if (oram_init(&opagedmem->oram, OPAGEDMEM_PAGE_SIZE,
                OPAGEDMEM_ORAM_BLOCKS_PER_BUCKET, num_blocks,
                oram_stash_size)) {
        /* Obliviousness violation - ORAM init failed. */
        goto exit;
    }

    opagedmem->first_level =
        calloc(OPAGEDMEM_FIRST_SIZE, sizeof(struct opagedmem_entry));
    if (!opagedmem->first_level) {
        /* Obliviousness violation - out of memory. */
        goto exit_destroy_oram;
    }

    /* This can technically be a malloc, but Valgrind complains about
     * uninitialized memory. */
    opagedmem->buffer = calloc(OPAGEDMEM_MID_COUNT, OPAGEDMEM_PAGE_SIZE);
    if (!opagedmem->buffer) {
        /* Obliviousness violation - out of memory. */
        goto exit_free_first_level;
    }

    /* This can technically be a calloc, but Valgrind complains about
     * uninitialized memory. */
    opagedmem->data_buffer = calloc(1, OPAGEDMEM_PAGE_SIZE);
    if (!opagedmem->data_buffer) {
        /* Obliviousness violation - out of memory. */
        goto exit_free_buffer;
    }

    return 0;

exit_free_buffer:
    free(opagedmem->buffer);
exit_free_first_level:
    free(opagedmem->first_level);
exit_destroy_oram:
    oram_destroy(&opagedmem->oram);
exit:
    return -1;
}

void opagedmem_destroy(opagedmem_t *opagedmem) {
    oram_destroy(&opagedmem->oram);
    free(opagedmem->first_level);
    free(opagedmem->buffer);
    free(opagedmem->data_buffer);
}

static uint64_t get_addr_block_id(uint64_t addr, int level) {
    uint64_t mask =
        ~((1lu <<
                    (64
                        - OPAGEDMEM_FIRST_BITS
                        - (level - 1) * OPAGEDMEM_MID_BITS))
                - 1);
    return (addr & mask) | level;
}

static int access_last_level(opagedmem_t *opagedmem, uint64_t addr,
        void *data, size_t data_size, size_t slice_start, size_t slice_size,
        bool write, struct opagedmem_entry *entry, bool is_real_access,
        uint64_t (*random_func)(void)) {
    /* Zero out the memory in the buffer. */
    memset(opagedmem->data_buffer, '\0', OPAGEDMEM_PAGE_SIZE);

    /* Read the page into the buffer. */
    uint64_t block_id = get_addr_block_id(addr, OPAGEDMEM_MID_COUNT + 1);
    if (oram_access(&opagedmem->oram, block_id, entry->leaf_id,
                opagedmem->data_buffer, false, &entry->leaf_id,
                entry->valid & is_real_access, random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    /* Perform the memory access. If this is a dummy set of accesses (read to a
     * non-allocated page), the zeros will be kept. */
    size_t offset = addr & OPAGEDMEM_OFFSET_MASK;
    o_slice(data, opagedmem->data_buffer, data_size, OPAGEDMEM_PAGE_SIZE,
            slice_start, offset, slice_size, write, is_real_access);

    /* Write the page back to ORAM. Perform a dummy read if this is not a
     * write. */
    if (oram_access(&opagedmem->oram, block_id, entry->leaf_id,
                opagedmem->data_buffer, write, &entry->leaf_id,
                (entry->valid | write) & is_real_access, random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    return 0;

exit:
    return -1;
}

/* Accesses the page table TABLE with NUM_ENTRIES entries, reads in the next
 * table into the buffer, and recurses to continue accessing deeper tables. If
 * IS_DUMMY is true, then this is a set of dummy accesses that ultimately won't
 * modify the page tables at all. */
static int access_level(opagedmem_t *opagedmem, uint64_t addr, void *data,
        size_t data_size, size_t slice_start, size_t slice_size, bool write,
        struct opagedmem_entry *table, size_t num_entries, int level,
        bool is_real_access, uint64_t (*random_func)(void)) {
    /* Compute the index in the table and find the table size. The if statement
     * depends only on the level, which is fixed. */
    size_t index;
    if (level == 0) {
        index = addr >> (64 - OPAGEDMEM_FIRST_BITS);
    } else {
        index =
            (addr
                >> (64 - OPAGEDMEM_FIRST_BITS - level * OPAGEDMEM_MID_BITS))
            & OPAGEDMEM_MID_MASK;
    }

    /* Oblivious scan through the table. */
    struct opagedmem_entry entry = {
        .leaf_id = random_func() % (1u << (opagedmem->oram.depth - 1)),
        .valid = false,
    };
    for (size_t i = 0; i < num_entries; i++) {
        bool cond = (i == index) & is_real_access;
        o_memcpy(&entry, &table[i], sizeof(entry), cond & table[i].valid);
    }

    /* Access the next level. The if statement depends only on the number of
     * levels, which is fixed. These calls should also modify the leaf ID in
     * the entry as part of the ORAM scheme. */
    if (level < OPAGEDMEM_MID_COUNT) {
        /* Read the page table into the buffer. Start initialized to 0 in case
         * this is a new page. If the entry isn't valid, perform a dummy access
         * since there's nothing to read. */
        uint64_t block_id = get_addr_block_id(addr, level + 1);
        memset(&opagedmem->buffer[level], '\0',
                sizeof(opagedmem->buffer[level]));
        if (oram_access(&opagedmem->oram, block_id, entry.leaf_id,
                    &opagedmem->buffer[level], false, &entry.leaf_id,
                    entry.valid & is_real_access, random_func)) {
            /* Obliviousness violation - access failed. */
            return -1;
        }

        if (access_level(opagedmem, addr, data, data_size, slice_start,
                    slice_size, write, opagedmem->buffer[level].entries,
                    OPAGEDMEM_MID_SIZE, level + 1, is_real_access,
                    random_func)) {
            /* Obliviousness violation - access failed. */
            goto exit;
        }

        /* Write the page table back to ORAM. */
        if (oram_access(&opagedmem->oram, block_id, entry.leaf_id,
                    &opagedmem->buffer[level], true, &entry.leaf_id,
                    (write | entry.valid) & is_real_access, random_func)) {
            /* Obliviousness violation - access failed. */
            goto exit;
        }
    } else {
        if (access_last_level(opagedmem, addr, data, data_size, slice_start,
                    slice_size, write, &entry, is_real_access, random_func)) {
            /* Obliviousness violation - access failed. */
            goto exit;
        }
    }

    /* Set the valid bit on the entry if this was a write. */
    o_setbool(&entry.valid, true, write);

    /* Oblivious write back to the table to update the entry. */
    o_select(&entry, table, num_entries, sizeof(entry), index, true,
            is_real_access);

    return 0;

exit:
    return -1;
}

int opagedmem_pageaccess(opagedmem_t *opagedmem, uint64_t addr, void *data,
        size_t size, bool write, bool is_real_access,
        uint64_t (*random_func)(void)) {
    size_t last_addr = addr + size - 1;
    if (((addr - (addr & OPAGEDMEM_OFFSET_MASK))
                != (last_addr - (last_addr & OPAGEDMEM_OFFSET_MASK)))
            & is_real_access) {
        /* Obliviousness violation - invalid call to opagedmem_access. */
        goto exit;
    }

    /* Begin recursively accessing the table. */
    if (access_level(opagedmem, addr, data, size, 0, size, write,
                opagedmem->first_level, OPAGEDMEM_FIRST_SIZE, 0, is_real_access,
                random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    return 0;

exit:
    return -1;
}

int opagedmem_access(opagedmem_t *opagedmem, uint64_t addr, void *data_,
        size_t size, bool write, bool is_real_access,
        uint64_t (*random_func)(void)) {
    unsigned char *data = data_;
    int ret;

    /* Access each page spanned by the accessed address range. */
    // TODO This could be optimized: If the access spans, for example, 3 pages,
    // we know that at least 2 of those pages will differ in at most one level
    // of the page table (it's impossible for 3 adjacent pages to be located in
    // 3 different second-level page tables, so we could optimize the access to
    // avoid the redundant accesess.
    size_t num_page_accesses = CEIL_DIV(size - 1, OPAGEDMEM_PAGE_SIZE) + 1;
    size_t start_page = addr - (addr & OPAGEDMEM_OFFSET_MASK);
    for (size_t i = 0; i < num_page_accesses; i++) {
        uint64_t page_start =
            o_maxul(addr, start_page + i * OPAGEDMEM_PAGE_SIZE);
        uint64_t page_end =
            o_minul(addr + size, start_page + (i + 1) * OPAGEDMEM_PAGE_SIZE);
        uint64_t page_size = 0;
        o_set64(&page_size, page_end - page_start, page_end >= page_start);
        bool page_is_real_access = is_real_access & (page_size > 0);

        ret =
            access_level(opagedmem, page_start, data, size, page_start - addr,
                    page_size, write, opagedmem->first_level,
                    OPAGEDMEM_FIRST_SIZE, 0, page_is_real_access, random_func);
        if (ret) {
            /* Obliviousness violation - page access failed. */
            goto exit;
        }

        addr += page_size;
        data += page_size;
        size -= page_size;
    }

    ret = 0;

exit:
    return ret;
}
