/**
 * mm.c
 *
 *  | prologue | header payload footer | epilogue |
 *
 *  Use implicit free list and first fit algorithm
 *
 */
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdint.h>
 
 #include "mm.h"
 #include "memlib.h"
 #include "config.h"
 
 /*********************************************************
  * NOTE TO STUDENTS: Before you do anything else, please
  * provide your team information in the following struct.
  ********************************************************/
 team_t team = {
         /* Team name */
         "ateam",
         /* First member's full name */
         "Harry Bovik",
         /* First member's email address */
         "bovik@cs.cmu.edu",
         /* Second member's full name (leave blank if none) */
         "",
         /* Second member's email address (leave blank if none) */
         ""
 };
 
 /* single word (4) or double word (8) alignment */
 #define ALIGNMENT 8
 
 /* rounds up to the nearest multiple of ALIGNMENT */
 #define ALIGN(size) ((((size) + (ALIGNMENT-1)) & ~0x7))
 
 /* header/footer size of each block */
 #define WORD_SIZE 4
 
 /* mask to calculate block size */
 #define SIZE_MASK (~0x7)
 
 /* get word at address */
 #define GET_WORD_AT(p) (*((uint32_t *) (p)))
 
 /* set word at address */
 #define SET_WORD_AT(p, value) (*((uint32_t *)(p)) = (value))
 
 /* get size of block from header or footer at address */
 #define GET_SIZE_AT(p) (GET_WORD_AT(p) & SIZE_MASK)
 
 /* set size of block to header or footer at address
  * and mark block as used.
  */
 #define SET_SIZE_AT(p, size) (SET_WORD_AT(p, ((size) | 0x1)))
 
 /*  check the flag bit of a header or footer at the address
  *  to see whether the block is used
  */
 #define IS_USED_AT(p) ((GET_WORD_AT(p) & 0x1) == 1)
 
 #define NIL ((void *) -1)
 
 static char *heap_start; // start address of heap, after prologue
 
 static char *heap_ceil; // ceil address of heap, the address of epilogue
 
 void validate(char *header) {
     char *footer = GET_SIZE_AT(header) + header - WORD_SIZE;
 
     uint32_t header_w = GET_WORD_AT(header);
     uint32_t footer_w = GET_WORD_AT(footer);
 
     if (header_w != footer_w) {
         printf("\n^^^^^^^^^^^^^%p(%u) %p(%u)^^^^^^^^^^^^^^^^^^^\n", (void *) header, header_w, (void *) footer,
                footer_w);
     }
 }
 
 void mm_check() {
 
     char *header = heap_start;
 
     printf(" | %p", (void *) heap_start);
 
     uint32_t size;
     while (header < heap_ceil) {
 
         validate(header);
 
         size = GET_SIZE_AT(header);
 
         printf(" | (%p-%p:%u)%s", (void *) header,
                (void *) (header + size - WORD_SIZE),
                size, IS_USED_AT(header) ? "USED" : "FREE");
 
         header += size;
     }
 
     printf(" | %p\n", (void *) heap_ceil);
 }
 
 /* 
  * mm_init - initialize the malloc package.
  * 
  * | prologue | header payload footer | epilogue |
  */
 int mm_init(void) {
     uint32_t init_size = WORD_SIZE * 2;
 
     heap_start = (char *) mem_sbrk(ALIGN(init_size));
     heap_ceil = heap_start + init_size;
 
     /* prologue */
     SET_SIZE_AT(heap_start, 0);
 
     /* epilogue */
     SET_SIZE_AT(heap_ceil - WORD_SIZE, 0);
 
     heap_start = heap_start + WORD_SIZE;
     heap_ceil = heap_ceil - WORD_SIZE;
 
     return 0;
 }
 
 
 /* 
  * mm_malloc - Allocate a block by incrementing the brk pointer.
  *     Always allocate a block whose size is a multiple of the alignment.
  */
 void *mm_malloc(size_t size) {
     uint32_t block_size = ALIGN(size + WORD_SIZE * 2);
     uint32_t free_size;
 
     int free_remain; // IMPORTANT !!!!!
 
     char *header = heap_start;
 
     while (header < heap_ceil) {
 //        validate(header);
 
         free_size = GET_SIZE_AT(header);
         free_remain = free_size - block_size;
 
         if (!IS_USED_AT(header) && (free_remain >= 0)) {
             break;
         }
 
         header = header + free_size;
 
     }
 
     if (header >= heap_ceil) { // use mem_sbrk
 
         char *last_footer = heap_ceil - WORD_SIZE;
 
         if (IS_USED_AT(last_footer)) {
             void *ptr = mem_sbrk(block_size);
             if (ptr == NIL) {
                 return NIL;
             }
 
             header = heap_ceil;
             heap_ceil = header + block_size;
 
             SET_SIZE_AT(header, block_size);
             SET_SIZE_AT(heap_ceil - WORD_SIZE, block_size);
 
             SET_SIZE_AT(heap_ceil, 0);
 
         } else { // combine last footer
             uint32_t last_size = GET_SIZE_AT(last_footer);
             char *header_last = last_footer - last_size + WORD_SIZE;
 
             uint32_t required = block_size - last_size;
 
             void *ptr = mem_sbrk(required);
             if (ptr == NIL) {
                 return NIL;
             }
 
             header = header_last;
             heap_ceil = header + block_size;
 
             char *new_footer = heap_ceil - WORD_SIZE;
 
             SET_SIZE_AT(header, block_size);
             SET_SIZE_AT(new_footer, block_size);
 
             SET_SIZE_AT(heap_ceil, 0);
 
         }
 
         return (void *) (header + WORD_SIZE);
     }
 
 
     char *old_footer, *new_header, *new_footer;
 
     old_footer = header + free_size - WORD_SIZE;
     new_header = header + block_size;
     new_footer = new_header - WORD_SIZE;
 
     SET_SIZE_AT(header, block_size);
     SET_SIZE_AT(new_footer, block_size);
 
     if (free_remain > 0) {
         SET_WORD_AT(new_header, free_remain);
         SET_WORD_AT(old_footer, free_remain);
     }
 
 //        printf("\n ALLOC %d ", size);
 //        mm_check();
 
 
     return (void *) (header + WORD_SIZE);
 
 }
 
 /*
  * mm_free - Freeing a block does nothing.
  */
 void mm_free(void *block) {
     char *header, *header_next, *header_pre, *footer, *footer_next, *footer_pre;
     uint32_t block_size, pre_size, next_size, real_size;
 
     header = block - WORD_SIZE;
     if (!IS_USED_AT(header))
         return;
 
     block_size = GET_SIZE_AT(header);
     header_next = header + block_size;
     footer = header_next - WORD_SIZE;
     footer_pre = header - WORD_SIZE;
 
     next_size = GET_SIZE_AT(header_next);
     pre_size = GET_SIZE_AT(footer_pre);
 
     footer_next = header_next + next_size - WORD_SIZE;
     header_pre = footer_pre - pre_size + WORD_SIZE;
 
 
     if (IS_USED_AT(footer_pre) && IS_USED_AT(header_next)) {
         real_size = block_size;
 
         SET_WORD_AT(header, real_size);
         SET_WORD_AT(footer, real_size);
 
     } else if (IS_USED_AT(footer_pre) && !IS_USED_AT(header_next)) {
 
         real_size = block_size + next_size;
 
         SET_WORD_AT(header, real_size);
         SET_WORD_AT(footer_next, real_size);
 
     } else if (!IS_USED_AT(footer_pre) && IS_USED_AT(header_next)) {
 
         real_size = block_size + pre_size;
 
         SET_WORD_AT(header_pre, real_size);
         SET_WORD_AT(footer, real_size);
 
     } else {
         real_size = block_size + pre_size + next_size;
 
         SET_WORD_AT(header_pre, real_size);
         SET_WORD_AT(footer_next, real_size);
     }
 
 //    printf("\n FREE %p", block - WORD_SIZE);
 //    mm_check();
 
 }
 
 /*
  * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
  */
 void *mm_realloc(void *block, size_t size) {
     if (block == NULL) {
         return mm_malloc(size);
     }
 
     char *header = block - WORD_SIZE;
 
     uint32_t old_size = GET_SIZE_AT(header);
     uint32_t new_size = ALIGN(size + WORD_SIZE * 2);
 
     if (!IS_USED_AT(header)) {
         return NIL;
     }
 
     if (size == 0) {
         mm_free(block);
         return mm_malloc(size);
     }
 
     if (new_size > old_size) {
         int missing = new_size - old_size;
         char *next_block = block + old_size;
 
         uint32_t next_size = GET_SIZE_AT(next_block - WORD_SIZE);
         int remain_size = next_size - missing;
 
         if (!IS_USED_AT(next_block - WORD_SIZE) && (remain_size >= 0)) {
             char *new_footer = block + new_size - WORD_SIZE * 2;
             char *new_header = new_footer + WORD_SIZE;
             char *next_footer = next_block + next_size - WORD_SIZE * 2;
 
             SET_SIZE_AT(header, new_size);
             SET_SIZE_AT(new_footer, new_size);
 
             if (remain_size > 0) {
                 SET_WORD_AT(new_header, remain_size);
                 SET_WORD_AT(next_footer, remain_size);
             }
             return block;
 
         }
 
         char *new_block = (char *) mm_malloc(size);
 
         memcpy(new_block, block, old_size - WORD_SIZE * 2); // copy payload
         mm_free(block);
 
         return new_block;
     }
 
     if (new_size < old_size) {
 
         int remain = old_size - new_size;
 
         char *old_header = header;
         char *new_header = old_header + new_size;
         char *new_footer = new_header - WORD_SIZE;
         char *old_footer = block + old_size - WORD_SIZE * 2;
 
         SET_SIZE_AT(old_header, new_size);
         SET_SIZE_AT(new_footer, new_size);
 
 
         SET_WORD_AT(new_header, remain);
         SET_WORD_AT(old_footer, remain);
 
         return block;
     }
 
     return block;
 }
 