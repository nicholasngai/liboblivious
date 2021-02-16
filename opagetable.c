#include "liboblivious/opagetable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "liboblivious/oram.h"
#include "liboblivious/primitives.h"

static int access_level(opagetable_t *opagetable, uint64_t addr, void *data,
        size_t size, bool write, bool *found, struct opagetable_entry *table,
        size_t num_entries, int level, uint64_t (*random_func)(void));
static int access_last_level(opagetable_t *opagetable, uint64_t addr,
        void *data, size_t size, bool write, bool *found,
        struct opagetable_entry *entry, uint64_t (*random_func)(void));

int opagetable_init(opagetable_t *opagetable, size_t num_bytes) {
    size_t num_blocks = (num_bytes + OPAGETABLE_PAGE_SIZE - 1)
        / OPAGETABLE_PAGE_SIZE;
    if (oram_init(&opagetable->oram, OPAGETABLE_PAGE_SIZE,
                OPAGETABLE_ORAM_BLOCKS_PER_BUCKET, num_blocks,
                OPAGETABLE_ORAM_STASH_SIZE)) {
        /* Obliviousness violation - ORAM init failed. */
        goto exit;
    }

    opagetable->first_level =
        calloc(OPAGETABLE_FIRST_SIZE, sizeof(struct opagetable_entry));
    if (!opagetable->first_level) {
        /* Obliviousness violation - out of memory. */
        goto exit_destroy_oram;
    }

    /* This can technically be a malloc, but Valgrind complains about
     * uninitialized memory. */
    opagetable->buffer = calloc(OPAGETABLE_MID_COUNT, OPAGETABLE_PAGE_SIZE);
    if (!opagetable->buffer) {
        /* Obliviousness violation - out of memory. */
        goto exit_free_first_level;
    }

    /* This can technically be a calloc, but Valgrind complains about
     * uninitialized memory. */
    opagetable->data_buffer = calloc(1, OPAGETABLE_PAGE_SIZE);
    if (!opagetable->data_buffer) {
        /* Obliviousness violation - out of memory. */
        goto exit_free_buffer;
    }

    opagetable->next_block_id = 0;

    return 0;

exit_free_buffer:
    free(opagetable->buffer);
exit_free_first_level:
    free(opagetable->first_level);
exit_destroy_oram:
    oram_destroy(&opagetable->oram);
exit:
    return -1;
}

void opagetable_destroy(opagetable_t *opagetable) {
    oram_destroy(&opagetable->oram);
    free(opagetable->first_level);
    free(opagetable->buffer);
    free(opagetable->data_buffer);
}

int opagetable_access(opagetable_t *opagetable, uint64_t addr, void *data,
        size_t size, bool write, bool *found, uint64_t (*random_func)(void)) {
    /* Initialize found to true. We will set it to false if any page table does
     * not contain the address. */
    *found = true;

    /* Begin recursively accessing the table. */
    if (access_level(opagetable, addr, data, size, write, found,
            opagetable->first_level, OPAGETABLE_FIRST_SIZE, 0, random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    return 0;

exit:
    return -1;
}

/* Accesses the page table TABLE with NUM_ENTRIES entries, reads in the next
 * table into the buffer, and recurses to continue accessing deeper tables. */
static int access_level(opagetable_t *opagetable, uint64_t addr, void *data,
        size_t size, bool write, bool *found, struct opagetable_entry *table,
        size_t num_entries, int level, uint64_t (*random_func)(void)) {
    bool curr_level_found = *found;

    /* Compute the index in the table and find the table size. The if statement
     * depends only on the level, which is fixed. */
    size_t index;
    if (level == 0) {
        index = addr >> (64 - OPAGETABLE_FIRST_BITS);
    } else {
        index = (addr
                >> (64 - OPAGETABLE_FIRST_BITS
                    - (level + 1) * OPAGETABLE_MID_BITS))
            & OPAGETABLE_MID_MASK;
    }

    /* Oblivious scan through the table. The block ID is iniitalized to the
     * next available block ID, which will be used if this is a write. The leaf
     * ID is initialized to a random value for a dummy access, which remains
     * the value if the entry is invalid. */
    struct opagetable_entry entry = {
        .block_id = opagetable->next_block_id,
        .leaf_id = random_func() % (1u << (opagetable->oram.depth - 1)),
        .valid = false,
    };
    opagetable->next_block_id++;
    for (size_t i = 0; i < num_entries; i++) {
        bool cond = i == index;
        o_memcpy(&entry, &table[i], sizeof(entry), cond & table[i].valid);
        o_setbool(&curr_level_found, false, cond & !table[i].valid);
    }
    *found &= curr_level_found;

    /* Access the next level. The if statement depends only on the number of
     * levels, which is fixed. These calls should also modify the leaf ID in
     * the entry as part of the ORAM scheme. */
    if (level < OPAGETABLE_MID_COUNT) {
        /* Read the page table into the buffer. Start initialized to 0 in case
         * this is a new page. */
        memset(&opagetable->buffer[level], '\0',
                sizeof(opagetable->buffer[level]));
        if (!write & curr_level_found
                & oram_access(&opagetable->oram, entry.block_id, entry.leaf_id,
                    &opagetable->buffer[level], false, &entry.leaf_id,
                    random_func)) {
            /* Obliviousness violation - access failed. */
            return -1;
        }

        if (access_level(opagetable, addr, data, size, write, found,
                    opagetable->buffer[level].entries, OPAGETABLE_MID_SIZE,
                    level + 1, random_func)) {
            /* Obliviousness violation - access failed. */
            goto exit;
        }

        /* Write the page table back to ORAM. Perform a dummy read if this is a
         * read and we didn't find the address. */
        if (!write & curr_level_found
                & oram_access(&opagetable->oram, entry.block_id, entry.leaf_id,
                    &opagetable->buffer[level], write | curr_level_found,
                    &entry.leaf_id, random_func)) {
            /* Obliviousness violation - access failed. */
            goto exit;
        }
    } else {
        if (access_last_level(opagetable, addr, data, size, write, found,
                    &entry, random_func)) {
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

static int access_last_level(opagetable_t *opagetable, uint64_t addr,
        void *data, size_t size, bool write, bool *found,
        struct opagetable_entry *entry, uint64_t (*random_func)(void)) {
    /* Read the page into the buffer. */
    if (!write & *found
            & oram_access(&opagetable->oram, entry->block_id, entry->leaf_id,
                opagetable->data_buffer, false, &entry->leaf_id, random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    /* Perform the memory access. */
    size_t offset = addr & OPAGETABLE_OFFSET_MASK;
    o_memaccess(data, opagetable->data_buffer + offset, size, write,
            write | *found);

    /* Write the page back to ORAM. Perform a dummy read if this is not a
     * write. */
    if (!write & *found
            & oram_access(&opagetable->oram, entry->block_id, entry->leaf_id,
                opagetable->data_buffer, write | *found, &entry->leaf_id,
                random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    return 0;

exit:
    return -1;
}
