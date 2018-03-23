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

/* find the needed free block in the free_list_heads */
bud_free_block *find_free(uint64_t t_order);
/* splitting block recursively; in every recursive call put the right buddy to the free_list_heads */
bud_free_block *split_block(bud_free_block *fblock, uint64_t t_order);
/* find the right order for the total block size */
uint64_t get_order(uint32_t tsize);
/* insert the right buddy into the free_list_heads */
void insert_free_list(bud_free_block *free_block);
/* cast the selected free block to allocated block */
bud_header *free_to_allocated(bud_free_block *free_block, uint64_t t_order, uint32_t tsize, uint32_t rsize);
/* verify if it is a valid pointer to free */
int valid_bud_ptr(void *ptr);
/* mark the block as free */
void mark_free(bud_header *freed_block);
/* coalescing block and recursively removing buddy from the free_list_heads */
bud_free_block *coalesce_block(bud_header *freed_block);
/* remove free buddy's header from free_list_heads */
void remove_header(bud_free_block *free_buddy);


/*
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in budmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
extern bud_free_block free_list_heads[NUM_FREE_LIST];

void *bud_malloc(uint32_t rsize) {
    /* check if the requested size is valid
    2 cases */
    uint32_t hsize = sizeof(bud_header); //the header size
    uint32_t max_size = MAX_BLOCK_SIZE - hsize;
    //case 0: resize=0; set errno to EINVAL and return NULL
    //case 1: it is greater than MAX_BLOCK_SIZE - sizeof(bud_header); set errno to EINVAL and return NULL
    if (rsize == 0 || rsize > max_size) {
        errno = EINVAL;
        return NULL;
    }
    /* if the requested size is valid then try to allocate */
    bud_free_block *best_block; // the best valid block to be allocated
    uint32_t tsize = hsize + rsize; //total needed size; header size + requested size
    //printf("The total size is: %u\n", tsize);
    //find the suitable free block
    uint64_t t_order = get_order(tsize); //get the best enough order
    //printf("The order needed is: %lu\n", t_order);
    bud_free_block *fblock = find_free(t_order);
    //if there is a valid free block then allocate it to user
    if (fblock != NULL) {
      //printf("There is valid free block in the free_list_heads!\n");
      //remove the header from the free_list_heads
      remove_header(fblock);
      best_block = split_block(fblock, t_order);
      //cast the bud_free_block to bud_header
      //modify the header of that block to allocated
      bud_header *best_block_header = free_to_allocated(best_block, t_order, tsize, rsize);
      //printf("The allocated block's order is: %u\n", best_block_header->order);
      return (best_block_header + 1);
    }
    //if no valid free block in the free list call bud_sbrk
    //todo after calling the bub_sbrk split the largest block to get an appropriate block
    //add the unused free block to free list heads
    else {
      void *increment_req = bud_sbrk();
      //fail to increment the break point
      if (increment_req == (void*) -1)
        return NULL;
      //increment break point successfully
      else {
        /*
        printf("The heap is increased to the break point: %p\n", increment_req);
        printf("Now the heap starts at the address: %p\n", bud_heap_start());
        printf("Now the heap ends at the address: %p\n", bud_heap_end());
        */
        //put the old break point address into the bud_free_block and set the next and prev to NULL
        bud_free_block *new_block = (bud_free_block*) increment_req;
        (new_block->header).order = ORDER_MAX-1;
        //printf("The new block's allocated is init to: %u\n", ((new_block->header).allocated));
        //printf("The new block's padded is init to: %u\n", ((new_block->header).padded = 0));
        /*the following are all Initialize correctlly
        (new_block->header).allocated = 0;
        (new_block->header).padded = 0;
        (new_block->header).rsize = 0;
        new_block->prev = NULL;
        new_block->next = NULL;
        */
        best_block = split_block(new_block, t_order);
        //printf("The best free block's address is: %p\n", best_block);
        bud_header *best_block_header = free_to_allocated(best_block, t_order, tsize, rsize);
        /*
        printf("The allocated block's order is: %u\n", best_block_header->order);
        printf("The address allocated is: %p\n", (best_block_header + 1));
        printf("The allocated address header is: %p\n", best_block_header);
        printf("================================\n");
        */
        return (best_block_header + 1);
      }
    }

    return NULL;
}

void *bud_realloc(void *ptr, uint32_t rsize) {
    //verify if the pointer and requested size passed in is valid
    if (rsize > (MAX_BLOCK_SIZE-sizeof(bud_header))) {
      errno = EINVAL;
      return NULL;
    }
    if (valid_bud_ptr(ptr)) {
      //printf("Calling abort\n");
      abort();
    }
    if (rsize == 0) {
      bud_free(ptr);
      return NULL;
    }
    if (ptr == NULL)
      return bud_malloc(rsize);
    //maybe senseless
    if (ptr == NULL && rsize == 0) {
      errno = EINVAL;
      return NULL;
    }
    //calculate the new total size; dealing with 3 cases
    uint32_t total_size = rsize + sizeof(bud_header);
    uint64_t req_order = get_order(total_size);// get the best enough order
    bud_header *ptr_header = (bud_header*) ptr - 1; // the address of the pointer's header
    uint64_t ptr_order = ptr_header->order;
    //case 0: the new total size is the same as the current block size
    if (req_order == ptr_order) {
      //set the header to hold the rsize
      ptr_header->rsize = rsize;
      return ptr;
    }
    //case 1: the new total size is bigger than the current block size
    if (req_order > ptr_order) {
      void *new_ptr = bud_malloc(rsize); //allocate new enough space
      //copy the old data to the new block
      uint64_t old_size = ptr_header->rsize;
      memcpy(new_ptr, ptr, old_size);
      bud_free(ptr); // free the old block
      return new_ptr;
    }
    //case 2: the new total size is smaller than the current block size
    if (req_order < ptr_order) {
      bud_free_block *new_ptr_header = split_block((bud_free_block*)ptr_header, req_order);
      //update the new header
      bud_header *new_header = free_to_allocated(new_ptr_header, req_order, total_size, rsize);
      void *new_ptr = new_header + 1;
      return new_ptr;
    }

    return NULL;
}

void bud_free(void *ptr) {
    //verify the ptr belongs to an allocated block
    //if it is a valid pointer
    int verifi_res = valid_bud_ptr(ptr);
    //printf("The verification result is: %d\n", verifi_res);
    if (!verifi_res) {
      bud_header *ptr_header = (bud_header*) ptr - 1; // get the header of the pointer
      //mark as free
      mark_free(ptr_header);
      //coalescing its buddy recursively and remove buddy's header from the free_list_heads
      bud_free_block *free_block = coalesce_block(ptr_header);
      //add the newly freed block to the free_list_heads
      insert_free_list(free_block);
    } else {
      //call abort
      //printf("Calling abort\n");
      abort();
    }

    return;
}


/*
The following functions for bud_malloc
*/

/* find the needed free block in the free_list_heads */
bud_free_block *find_free(uint64_t t_order) {
    int pos = t_order - ORDER_MIN;
    //printf("The position is: %d\n", pos);
    if (free_list_heads[pos].next != &free_list_heads[pos])
      return free_list_heads[pos].next;
    //iter through the free list heads
    for (int i=pos+1; i<NUM_FREE_LIST; i++) {
        //printf("It is the iteration: %d\n", i);
        //return the available block in the free_list_heads
        if (free_list_heads[i].next != &free_list_heads[i])
          return free_list_heads[i].next;
    }
    //if no suitable block return NULL
    return NULL;
}

/* splitting block recursively; in every recursive call put the right buddy to the free_list_heads */
bud_free_block *split_block(bud_free_block *fblock, uint64_t t_order) {
  //printf("The new block's address is: %p\n", fblock);
  //printf("The desired order is: %lu\n", t_order);
  //base case: the block has a valid order for allocation
  uint64_t block_order = (fblock->header).order;
  //printf("The current block's order is: %llu\n", block_order);
  if (t_order == block_order)
    return fblock;
  //split the block into halves and put the right buddy into the free_list_heads
  //uint32_t split_size = ORDER_TO_BLOCK_SIZE(block_order - 1);
  uint64_t split_size = ORDER_TO_BLOCK_SIZE(block_order - 1);
  bud_free_block *left_buddy_block = fblock, *right_buddy_block = NULL;
  //decrease left buddy's order by 1
  (left_buddy_block->header).order = block_order - 1;
  //find the new address for right buddy and set its order the same as left buddy
  //cast to uintptr_t for pointer arithmetic
  uintptr_t left_buddy_address = (uintptr_t) left_buddy_block;
  uintptr_t right_buddy_address;
  right_buddy_address = left_buddy_address^((uintptr_t)split_size);
  //
  /*cast to (char*) for pointer arithmetic
  char *left_buddy_address = (char*) left_buddy_block;
  char *right_buddy_address;
  right_buddy_address = left_buddy_address^((char*)split_size);
  */
  right_buddy_block = (bud_free_block*) right_buddy_address;
  (right_buddy_block->header).order = block_order - 1;
  //mark the right buddy as free
  mark_free(&(right_buddy_block->header));
  //put the right buddy into the free_list_heads
  insert_free_list(right_buddy_block);

  return split_block(left_buddy_block, t_order);
}

/* find the right order for the total block size */
uint64_t get_order(uint32_t tsize) {
    //tsize cannot be 0
    if (tsize <= MIN_BLOCK_SIZE)
      return ORDER_MIN;
    uint64_t count_order = 0;
    //tsize is a power of 2
    if (!(tsize&(tsize-1))) {
      while (tsize != 1) {
        count_order += 1;
        tsize >>= 1;
      }
    } else {
      while (tsize != 0) {
        count_order += 1;
        tsize >>= 1;
      }
    }

    return count_order;
}

/* insert the right buddy into the free_list_heads */
void insert_free_list(bud_free_block *free_block) {
    uint64_t block_order = (free_block->header).order;
    int pos = block_order - ORDER_MIN;
    free_block->next = free_list_heads[pos].next;
    (free_list_heads[pos].next)->prev = free_block;
    free_list_heads[pos].next = free_block;
    free_block->prev = &free_list_heads[pos];
}

/* cast the selected free block to allocated block */
bud_header *free_to_allocated(bud_free_block *best_block, uint64_t t_order, uint32_t tsize, uint32_t rsize) {
    ((bud_header*) best_block)->allocated = 1;
    ((bud_header*) best_block)->order = t_order;
    if (ORDER_TO_BLOCK_SIZE(t_order) > tsize)
      ((bud_header*) best_block)->padded = 1;
    else
      ((bud_header*) best_block)->padded = 0;
    ((bud_header*) best_block)->rsize = (uint64_t) rsize;

    return ((bud_header*) best_block);
}


/*
The following functions for bud_free
*/

/* verify if it is a valid pointer to free */
int valid_bud_ptr(void *ptr) {
    //printf("The pointer gonna be verified is: %p\n", ptr);
    //printf("The pointer's header is: %p\n", ((bud_header*) ptr-1));
    uintptr_t int_ptr = (uintptr_t) ptr; // cast to int for easier comparing
    bud_header *ptr_header = (bud_header*) ptr - 1; // the address of the pointer's header
    //case 0: prt must be in range(bud_heap_start, bud_heap_end)
    uintptr_t heap_lower_bound = (uintptr_t) bud_heap_start();
    uintptr_t heap_upper_bound = (uintptr_t) bud_heap_end();
    /*
    for lower bound we need a block for header
    for the uppoer bound, the bud_heap_end() will return the 1st address that beyong the current heap
    */
    if (int_ptr <= heap_lower_bound || int_ptr >= heap_upper_bound) {
        //printf("case 0\n");
        return 1;
    }
    //case 1: ptr must align to a multiple of 8; sizeof(bud_header)?
    uint64_t align_factor = sizeof(bud_header);
    if ((int_ptr%align_factor) != 0) {
        //printf("case 1\n");
        return 1;
    }
    //case 2: order must in range(ORDER_MIN, ORDER_MAX)
    uint64_t ptr_order = ptr_header->order;
    if (ptr_order < ORDER_MIN || ptr_order > ORDER_MAX) {
        //printf("case 2\n");
        //printf("The pointer's order is: %lu\n", ptr_order);
        return 1;
    }
    //case 3: header.allocated is 1
    if (ptr_header->allocated != 1) {
        //printf("case 3\n");
        return 1;
    }
    //case 4: padded consistency
    /*
    The padded bit in the header is 0, but requested_size + sizeof(bud_header) != (block size)
    The padded bit in the header is 1, but requested_size + sizeof(bud_header) == (block size)
    */
    uint64_t total_size = ptr_header->rsize + sizeof(bud_header);
    uint64_t block_size = ORDER_TO_BLOCK_SIZE(ptr_header->order);
    if (ptr_header->padded == 0 && total_size != block_size) {
        //printf("case 4\n");
        return 1;
    }
    if (ptr_header->padded == 1 && total_size == block_size) {
        //printf("case 4\n");
        return 1;
    }
    //case 5: requested size is consistent with the order
    uint64_t consistent_order = get_order((ptr_header->rsize)+sizeof(bud_header));
    if (consistent_order != ptr_order) {
        //printf("case 5\n");
        return 1;
    }

    //if all conditions a satisfied then return true
    return 0;
}

/* mark the block as free */
void mark_free(bud_header *freed_block) {
    freed_block->allocated = 0;
    freed_block->padded = 0;
    freed_block->rsize = 0;
}

/* coalescing block and recursively removing buddy from the free_list_heads */
/* just for buddies A^S = B
   sizes have to be same
*/
bud_free_block *coalesce_block(bud_header *freed_block) {
    //base case reaching the maximum order
    if (freed_block->order == (ORDER_MAX-1))
      return ((bud_free_block*) freed_block);
    //find the buddy for the just freed block
    //check if it is the left or right buddy
    bud_header *buddy;
    bud_free_block *free_buddy;
    uint64_t freed_block_order = freed_block->order;
    uint64_t block_size = ORDER_TO_BLOCK_SIZE(freed_block_order);
    //cast to uintptr_t for pointer arithmetic
    uintptr_t buddy_address;
    uintptr_t freed_block_address = (uintptr_t) freed_block;
    buddy_address = freed_block_address^((uintptr_t)block_size);
    //
    /*cast to (char*) for pointer arithmetic
    char *buddy_address;
    char *freed_block_address = (char*) freed_block;
    buddy_address = freed_block_address^((char*)block_size);
    */
    buddy = (bud_header*) buddy_address;
    //base case the freed block's buddy is not free
    //or the buddy & the block have different size
    //or reaching the maximum order
    if ((buddy->allocated == 1) || (buddy->order != freed_block->order) || (buddy->order == (ORDER_MAX-1))) {
      return ((bud_free_block*) freed_block);
    }
    //if the buddy is free and has the same size as the free block
    //cast the buddy header to bud_free_block
    free_buddy = (bud_free_block*) buddy; //it shoul be in the free_list_heads
    //for left buddy
    if (buddy_address < freed_block_address) {
      //remove the left buddy from the free_list_heads
      remove_header(free_buddy);
      //change the order of left buddy
      buddy->order += 1;
      //recursive call
      return coalesce_block(buddy);
    }
    //for right buddy
    //else if (buddy > freed_block) {
    else {
      //remove the right buddy from the free_list_heads
      remove_header(free_buddy);
      //change the order of freed_block
      freed_block->order += 1;
      //recursive call
      return coalesce_block(freed_block);
    }
}

/* remove free buddy's header from free_list_heads */
void remove_header(bud_free_block *free_buddy) {
    free_buddy->prev->next = free_buddy->next;
    free_buddy->next->prev = free_buddy->prev;
    free_buddy->next = NULL;
    free_buddy->prev = NULL;
}

/*
The following functions for bud_realloc
*/
