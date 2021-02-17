#include "liboblivious/opagedmem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "liboblivious/oram.h"
#include "liboblivious/primitives.h"

static int access_level(opagedmem_t *opagedmem, uint64_t addr, void *data,
        size_t size, bool write, bool *found, struct opagedmem_entry *table,
        size_t num_entries, int level, uint64_t (*random_func)(void));
static int access_last_level(opagedmem_t *opagedmem, uint64_t addr,
        void *data, size_t size, bool write, bool *found,
        struct opagedmem_entry *entry, uint64_t (*random_func)(void));

int opagedmem_init(opagedmem_t *opagedmem, size_t num_bytes) {
    size_t num_blocks = (num_bytes + OPAGEDMEM_PAGE_SIZE - 1)
        / OPAGEDMEM_PAGE_SIZE;
    if (oram_init(&opagedmem->oram, OPAGEDMEM_PAGE_SIZE,
                OPAGEDMEM_ORAM_BLOCKS_PER_BUCKET, num_blocks,
                OPAGEDMEM_ORAM_STASH_SIZE)) {
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
        size_t size, bool write, bool *found, uint64_t (*random_func)(void)) {
    /* Initialize found to true. We will set it to false if any page table does
     * not contain the address. */
    *found = true;

    /* Begin recursively accessing the table. */
    if (access_level(opagedmem, addr, data, size, write, found,
            opagedmem->first_level, OPAGEDMEM_FIRST_SIZE, 0, random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    return 0;

exit:
    return -1;
}

/* Accesses the page table TABLE with NUM_ENTRIES entries, reads in the next
 * table into the buffer, and recurses to continue accessing deeper tables. */
static int access_level(opagedmem_t *opagedmem, uint64_t addr, void *data,
        size_t size, bool write, bool *found, struct opagedmem_entry *table,
        size_t num_entries, int level, uint64_t (*random_func)(void)) {
    bool curr_level_found = *found;

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
        bool cond = i == index;
        o_memcpy(&entry, &table[i], sizeof(entry), cond & table[i].valid);
        o_setbool(&curr_level_found, false, cond & !table[i].valid);
    }
    *found &= curr_level_found;

    /* Access the next level. The if statement depends only on the number of
     * levels, which is fixed. These calls should also modify the leaf ID in
     * the entry as part of the ORAM scheme. */
    if (level < OPAGEDMEM_MID_COUNT) {
        /* Read the page table into the buffer. Start initialized to 0 in case
         * this is a new page. */
        memset(&opagedmem->buffer[level], '\0',
                sizeof(opagedmem->buffer[level]));
        if (!write & curr_level_found
                & oram_access(&opagedmem->oram, entry.block_id, entry.leaf_id,
                    &opagedmem->buffer[level], false, &entry.leaf_id,
                    random_func)) {
            /* Obliviousness violation - access failed. */
            return -1;
        }

        if (access_level(opagedmem, addr, data, size, write, found,
                    opagedmem->buffer[level].entries, OPAGEDMEM_MID_SIZE,
                    level + 1, random_func)) {
            /* Obliviousness violation - access failed. */
            goto exit;
        }

        /* Write the page table back to ORAM. Perform a dummy read if this is a
         * read and we didn't find the address. */
        if (!write & curr_level_found
                & oram_access(&opagedmem->oram, entry.block_id, entry.leaf_id,
                    &opagedmem->buffer[level], write | curr_level_found,
                    &entry.leaf_id, random_func)) {
            /* Obliviousness violation - access failed. */
            goto exit;
        }
    } else {
        if (access_last_level(opagedmem, addr, data, size, write, found, &entry,
                    random_func)) {
            /* Obliviousness violation - access failed. */
            goto exit;
        }
    }

    /* Set the valid bit on the entry if this was a write. */
    o_setbool(&entry.valid, true, write);

    /* Oblivious scan back through the table to update the entry. */
    for (size_t i = 0; i < num_entries; i++) {
        bool cond = i == index;
        o_memcpy(&table[i], &entry, sizeof(table[i]), cond & entry.valid);
    }

    return 0;

exit:
    return -1;
}

static int access_last_level(opagedmem_t *opagedmem, uint64_t addr,
        void *data, size_t size, bool write, bool *found,
        struct opagedmem_entry *entry, uint64_t (*random_func)(void)) {
    /* Read the page into the buffer. */
    if (!write & *found
            & oram_access(&opagedmem->oram, entry->block_id, entry->leaf_id,
                opagedmem->data_buffer, false, &entry->leaf_id, random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    /* Perform the memory access. */
    size_t offset = addr & OPAGEDMEM_OFFSET_MASK;
    o_memaccess(data, opagedmem->data_buffer + offset, size, write,
            write | *found);

    /* Write the page back to ORAM. Perform a dummy read if this is not a
     * write. */
    if (!write & *found
            & oram_access(&opagedmem->oram, entry->block_id, entry->leaf_id,
                opagedmem->data_buffer, write | *found, &entry->leaf_id,
                random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    return 0;

exit:
    return -1;
}
