#include "liboblivious/opagetable.h"
#include <stdio.h>
#include <stdlib.h>
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

    /* Oblivious scan through the table. The leaf ID is initialized to a random
     * value for a dummy access, which remains the value if the entry is
     * invalid. The block ID is iniitalized to 0 to appease Valgrind. */
    struct opagetable_entry entry;
    entry.block_id = 0;
    entry.leaf_id = random_func() % (1u << (opagetable->oram.depth - 1));
    for (size_t i = 0; i < num_entries; i++) {
        bool cond = i == index;
        o_memcpy(&entry, &table[i], sizeof(entry), cond & table[i].valid);
        o_setbool(found, false, cond & !table[i].valid);
    }

    /* Access the next level. The if statement depends only on the number of
     * levels, which is fixed. */
    if (level < OPAGETABLE_MID_COUNT) {
        /* Read the page into the buffer. */
        if (*found
                & oram_access(&opagetable->oram, entry.block_id, entry.leaf_id,
                    &opagetable->buffer[level], false, &entry.leaf_id,
                    random_func)) {
            /* Obliviousness violation - access failed. */
            return -1;
        }

        return access_level(opagetable, addr, data, size, write, found,
                opagetable->buffer[level].entries, OPAGETABLE_MID_SIZE,
                level + 1, random_func);
    } else {
        return access_last_level(opagetable, addr, data, size, write, found,
                &entry, random_func);
    }
}

static int access_last_level(opagetable_t *opagetable, uint64_t addr,
        void *data, size_t size, bool write, bool *found,
        struct opagetable_entry *entry, uint64_t (*random_func)(void)) {
    /* Read the page into the buffer. */
    if (*found & oram_access(&opagetable->oram, entry->block_id, entry->leaf_id,
            opagetable->data_buffer, false, &entry->leaf_id, random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    /* Perform the memory access. */
    size_t offset = addr & OPAGETABLE_OFFSET_MASK;
    o_memaccess(data, opagetable->data_buffer + offset, size, write, *found);

    /* Write the page back to the buffer. */
    if (oram_access(&opagetable->oram, entry->block_id, entry->leaf_id,
            opagetable->data_buffer, true, &entry->leaf_id, random_func)) {
        /* Obliviousness violation - access failed. */
        goto exit;
    }

    return 0;

exit:
    return -1;
}
