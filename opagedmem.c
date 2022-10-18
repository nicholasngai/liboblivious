#include "liboblivious/opagedmem.h"
#include <stdlib.h>
#include <string.h>
#include "liboblivious/oram.h"
#include "liboblivious/primitives.h"

static int access_level(opagedmem_t *opagedmem, uint64_t addr, void *data,
        size_t size, bool write, struct opagedmem_entry *table,
        size_t num_entries, int level, bool is_real_access,
        uint64_t (*random_func)(void));
static int access_last_level(opagedmem_t *opagedmem, uint64_t addr,
        void *data, size_t size, bool write, struct opagedmem_entry *entry,
        bool is_real_access, uint64_t (*random_func)(void));

int opagedmem_init(opagedmem_t *opagedmem, size_t num_bytes) {
    size_t num_blocks = (num_bytes + OPAGEDMEM_PAGE_SIZE - 1)
        / OPAGEDMEM_PAGE_SIZE;

    /* Use OPAGEDMEM_ORAM_STASH_SIZE as the persistent stash size (which
     * doesn't depend on num_bytes) + log2(num_bytes / OPAGEDMEM_PAGE_SIZE) *
     * OPAGEDMEM_ORAM_BLOCKS_PER_BUCKET * 2 as the transient stash size (which
     * depends on Z * log2(N)). The * 2 exists because the doubly oblivious
     * sort for the stash requires double the stash size. */
    size_t oram_stash_size = 0;
    while ((1u << oram_stash_size) < num_blocks) {
        oram_stash_size++;
    }
    oram_stash_size =
        OPAGEDMEM_ORAM_STASH_SIZE
            + oram_stash_size * OPAGEDMEM_ORAM_BLOCKS_PER_BUCKET * 2;

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

    opagedmem->next_block_id = 0;

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

int opagedmem_access(opagedmem_t *opagedmem, uint64_t addr, void *data,
        size_t size, bool write, bool is_real_access,
        uint64_t (*random_func)(void)) {
    /* Begin recursively accessing the table. */
    if (access_level(opagedmem, addr, data, size, write,
            opagedmem->first_level, OPAGEDMEM_FIRST_SIZE, 0, is_real_access,
            random_func)) {
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
        size_t size, bool write, struct opagedmem_entry *table,
        size_t num_entries, int level, bool is_real_access,
        uint64_t (*random_func)(void)) {
    /* Compute the index in the table and find the table size. The if statement
     * depends only on the level, which is fixed. */
    size_t index;
    if (level == 0) {
        index = addr >> (64 - OPAGEDMEM_FIRST_BITS);
    } else {
        index = (addr
                >> (64 - OPAGEDMEM_FIRST_BITS
                    - (level + 1) * OPAGEDMEM_MID_BITS))
            & OPAGEDMEM_MID_MASK;
    }

    /* Oblivious scan through the table. The block ID is iniitalized to the
     * next available block ID, which will be used if this is a write. The leaf
     * ID is initialized to a random value for a dummy access, which remains
     * the value if the entry is invalid. */
    struct opagedmem_entry entry = {
        .block_id = opagedmem->next_block_id,
        .leaf_id = random_func() % (1u << (opagedmem->oram.depth - 1)),
        .valid = false,
    };
    opagedmem->next_block_id++;
    for (size_t i = 0; i < num_entries; i++) {
        bool cond = (i == index) & is_real_access;
        o_memcpy(&entry, &table[i], sizeof(entry), cond & table[i].valid);
    }

    /* Access the next level. The if statement depends only on the number of
     * levels, which is fixed. These calls should also modify the leaf ID in
     * the entry as part of the ORAM scheme. */
    if (level < OPAGEDMEM_MID_COUNT - 1) {
        /* Read the page table into the buffer. Start initialized to 0 in case
         * this is a new page. If the entry isn't valid, perform a dummy access
         * since there's nothing to read. */
        memset(&opagedmem->buffer[level], '\0',
                sizeof(opagedmem->buffer[level]));
        if (oram_access(&opagedmem->oram, entry.block_id, entry.leaf_id,
                    &opagedmem->buffer[level], false, &entry.leaf_id,
                    entry.valid, random_func)) {
            /* Obliviousness violation - access failed. */
            return -1;
        }

        if (access_level(opagedmem, addr, data, size, write,
                    opagedmem->buffer[level].entries, OPAGEDMEM_MID_SIZE,
                    level + 1, is_real_access, random_func)) {
            /* Obliviousness violation - access failed. */
            goto exit;
        }

        /* Write the page table back to ORAM. */
        if (oram_access(&opagedmem->oram, entry.block_id, entry.leaf_id,
                    &opagedmem->buffer[level], true, &entry.leaf_id,
                    write | entry.valid, random_func)) {
            /* Obliviousness violation - access failed. */
            goto exit;
        }
    } else {
        if (access_last_level(opagedmem, addr, data, size, write, &entry,
                    is_real_access, random_func)) {
            /* Obliviousness violation - access failed. */
            goto exit;
        }
    }

    /* Set the valid bit on the entry if this was a write. */
    o_setbool(&entry.valid, true, write);

    /* Oblivious scan back through the table to update the entry. */
    for (size_t i = 0; i < num_entries; i++) {
        bool cond = (i == index) & is_real_access;
        o_memcpy(&table[i], &entry, sizeof(table[i]), cond);
    }

    return 0;

exit:
    return -1;
}

static int access_last_level(opagedmem_t *opagedmem, uint64_t addr,
        void *data, size_t size, bool write, struct opagedmem_entry *entry,
        bool is_real_access, uint64_t (*random_func)(void)) {
    /* Zero out the memory in the buffer. */
    memset(opagedmem->data_buffer, '\0', OPAGEDMEM_PAGE_SIZE);

    /* Read the page into the buffer. */
    if (oram_access(&opagedmem->oram, entry->block_id, entry->leaf_id,
                opagedmem->data_buffer, false, &entry->leaf_id, entry->valid,
                random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    /* Perform the memory access. If this is a dummy set of accesses (read to a
     * non-allocated page), the zeros will be kept. */
    size_t offset = addr & OPAGEDMEM_OFFSET_MASK;
    for (size_t i = 0; i < OPAGEDMEM_PAGE_SIZE; i++) {
        bool cond = (i >= offset) & (i < offset + size) & is_real_access;
        o_access8((unsigned char *) data + i - offset,
                (unsigned char *) opagedmem->data_buffer + i, write, cond);
    }

    /* Write the page back to ORAM. Perform a dummy read if this is not a
     * write. */
    if (oram_access(&opagedmem->oram, entry->block_id, entry->leaf_id,
                opagedmem->data_buffer, write, &entry->leaf_id,
                write & is_real_access, random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    return 0;

exit:
    return -1;
}
