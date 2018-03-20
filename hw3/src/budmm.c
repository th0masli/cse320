/*
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "budmm.h"

/*
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in budmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
extern bud_free_block free_list_heads[NUM_FREE_LIST];

void *bud_malloc(uint32_t rsize) {
    /* check if the requested size is valid
    2 cases */
    uint32_t max_rsize = MAX_BLOCK_SIZE - sizeof(bud_header);
    //case 0: resize=0; set errno to EINVAL and return NULL
    //case 1: it is greater than MAX_BLOCK_SIZE - sizeof(bud_header); set errno to EINVAL and return NULL
    if (rsize == 0 || rsize > max_rsize) {
        errno = EINVAL;
        return NULL
    }
    /* if the requested size is valid then try to allocate */
    //iter through the free list heads
    for (int i=0; i<NUM_FREE_LIST; i++) {
        //todo return the allocated pointer address
    }
    //if no valid free block in the free list call bud_sbrk
    //todo after calling the bub_sbrk split the largest block to get an appropriate block
    //add the unused free block to free list heads

    return NULL;
}

void *bud_realloc(void *ptr, uint32_t rsize) {
    return NULL;
}

void bud_free(void *ptr) {
    return;
}
