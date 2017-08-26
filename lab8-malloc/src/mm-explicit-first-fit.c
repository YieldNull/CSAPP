/**
 * mm.c
 *
 *  | prologue (as free block) | header payload footer | epilogue (as free block) |
 *
 *  Use explicit free list and first fit algorithm
 *
 *  Eliminate the footer in allocated blocks.
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

#define PTR_SIZE 4

/* mask to calculate block size */
#define SIZE_MASK (~0x7)

/* get word at address */
#define GET_WORD_AT(p) (*((uint32_t *) (p)))

/* set word at address */
#define SET_WORD_AT(p, value) (*((uint32_t *)(p)) = (value))

/* get size of block from header or footer at address */
#define GET_SIZE_AT(p) (GET_WORD_AT(p) & SIZE_MASK)

/*  check the flag bit of a header or footer at the address
 *  to see whether the block is used
 */
#define IS_USED_AT(p) ((GET_WORD_AT(p) & 0x1) == 1)

#define IS_PRE_USED(p) ((GET_WORD_AT(p) & 0b10) == 0b10)

#define SET_PRE_USED(p) SET_WORD_AT(p,GET_WORD_AT(p) | 0b10)

#define SET_PRE_FREE(p) SET_WORD_AT(p,GET_WORD_AT(p) & (~0b10))

#define GET_FREE_PRE(p) ((char *)(*(uint32_t *)((p) + WORD_SIZE)))

#define GET_FREE_NEXT(p) ((char *)(*(uint32_t *)((p) + WORD_SIZE + PTR_SIZE)))

#define SET_FREE_PRE(p, ptr) (*((uint32_t *) ((p) + WORD_SIZE)) = (uint32_t)(ptr))

#define SET_FREE_NEXT(p, ptr) (*((uint32_t *) ((p) + WORD_SIZE + PTR_SIZE)) = (uint32_t)(ptr))

#define NIL ((void *) -1)

#define FREE 0

#define USED 1

#define MIN_FREE_SIZE (WORD_SIZE * 2 + PTR_SIZE * 2)

static char *heap_start; // start address of heap, after prologue

static char *heap_ceil; // ceil address of heap, the address of epilogue

static char *free_start;

inline void set_header(char *header, uint32_t size, int pre_used, int used) {
    *((uint32_t *) (header)) = size | (pre_used << 1) | used;
}

/*
 * mm_init - initialize the malloc package.
 *
 * | prologue | header payload footer | epilogue |
 */
int mm_init(void) {
    mem_sbrk(WORD_SIZE); // for alignment

    uint32_t init_size = ALIGN(WORD_SIZE * 8);

    heap_start = (char *) mem_sbrk(init_size);
    heap_ceil = heap_start + init_size;


    char *prologure = heap_start;
    char *epilogue = heap_start + WORD_SIZE * 4;

    /* prologue as free block */
    set_header(prologure, 0, USED, USED);
    set_header(epilogue - WORD_SIZE, 0, USED, USED);

    /* epilogue as free block */
    set_header(epilogue, 0, USED, USED);
    set_header(epilogue + WORD_SIZE * 3, 0, USED, USED);

    heap_start = epilogue;
    heap_ceil = epilogue;

    free_start = prologure;

    SET_FREE_PRE(free_start, NIL);
    SET_FREE_NEXT(free_start, epilogue);

    SET_FREE_PRE(epilogue, free_start);
    SET_FREE_NEXT(epilogue, NIL);

    return 0;
}


void add_free(char *free) {

    char *ptr = GET_FREE_NEXT(free_start);

    while (ptr < free) {
        ptr = GET_FREE_NEXT(ptr);
    }


    // pre -> free -> ptr
    char *pre = GET_FREE_PRE(ptr);

    SET_FREE_PRE(free, pre);
    SET_FREE_NEXT(pre, free);

    SET_FREE_NEXT(free, ptr);
    SET_FREE_PRE(ptr, free);
}

char *find_free(uint32_t block_size) {

    char *ptr = GET_FREE_NEXT(free_start);

    while (ptr < heap_ceil) {
        if (GET_SIZE_AT(ptr) >= block_size) {
            return ptr;
        }

        ptr = GET_FREE_NEXT(ptr);
    }

    return NIL;
}

void link_free(char *pre, char *next) {
    SET_FREE_NEXT(pre, next);
    SET_FREE_PRE(next, pre);
}

void mm_check() {

    char *header = heap_start;

    printf(" | %p", (void *) heap_start);

    uint32_t size;
    while (header < heap_ceil) {


        size = GET_SIZE_AT(header);

        printf(" | PRE:%s(%p-%p:%u)%s", IS_PRE_USED(header) ? "USED" : "FREE",
               (void *) header,
               (void *) (header + size - WORD_SIZE),
               size, IS_USED_AT(header) ? "USED" : "FREE");

        header += size;
    }

    printf(" | PRE:%s %p\n", IS_PRE_USED(header) ? "USED" : "FREE",
           (void *) heap_ceil);

    char *ptr = free_start;

    while (ptr != NIL) {
        printf("%p -> ", (void *) ptr);
        ptr = GET_FREE_NEXT(ptr);
    }

    printf("\n");
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    uint32_t block_size = ALIGN(size + WORD_SIZE);

    char *header = find_free(block_size);

    if (header == NIL) { // use mem_sbrk
        void *ptr = mem_sbrk(block_size);
        if (ptr == NIL) {
            return NIL;
        }

        int is_pre_used = IS_PRE_USED(heap_ceil);
        char *free_pre = GET_FREE_PRE(heap_ceil);

        header = heap_ceil;
        heap_ceil = header + block_size;

        set_header(header, block_size, is_pre_used, USED);

        // heap ceil
        set_header(heap_ceil, 0, USED, USED);

        // relink free list tail
        link_free(free_pre, heap_ceil);

        SET_FREE_NEXT(heap_ceil, NIL);

    } else {

        uint32_t free_size = GET_SIZE_AT(header);
        uint32_t free_remain = free_size - block_size;

        char *free_pre = GET_FREE_PRE(header);
        char *free_next = GET_FREE_NEXT(header);

        if (free_remain >= MIN_FREE_SIZE) {
            char *old_footer = header + free_size - WORD_SIZE;
            char *new_header = header + block_size;

            set_header(header, block_size, USED, USED); // pre block of a free block must be used

            // gen new free block and link
            set_header(new_header, free_remain, USED, FREE);
            set_header(old_footer, free_remain, USED, FREE);

            link_free(free_pre, new_header);
            link_free(new_header, free_next);

        } else {
            // use the whole block

            set_header(header, free_size, USED, USED);

            SET_PRE_USED(header + free_size); // mark as used on the next block header

            // link
            link_free(free_pre, free_next);
        }
    }

//    printf("\nALLOC: %u ", block_size);
//    mm_check();

    return (void *) (header + WORD_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *block) {

    char *header = block - WORD_SIZE;
    if (!IS_USED_AT(header))
        return;

    uint32_t block_size = GET_SIZE_AT(header);

    int is_pre_used = IS_PRE_USED(header);
    int is_next_used = IS_USED_AT(header + block_size);

    char *next_header = header + block_size;

    if (is_pre_used && is_next_used) {

        set_header(header, block_size, USED, FREE);
        set_header(next_header - WORD_SIZE, block_size, USED, FREE);

        SET_PRE_FREE(next_header); // mark as free on the next block header

        add_free(header);

    } else if (is_pre_used && !is_next_used) {

        uint32_t next_size = GET_SIZE_AT(next_header);
        uint32_t new_size = block_size + next_size;

        char *next_footer = header + new_size - WORD_SIZE;

        set_header(header, new_size, USED, FREE);
        set_header(next_footer, new_size, USED, FREE);

        char *free_pre = GET_FREE_PRE(next_header);
        char *free_next = GET_FREE_NEXT(next_header);

        link_free(free_pre, header);
        link_free(header, free_next);

    } else if (!is_pre_used && is_next_used) {

        uint32_t pre_size = GET_SIZE_AT(header - WORD_SIZE);
        uint32_t new_size = pre_size + block_size;


        char *pre_header = header - pre_size;

        set_header(pre_header, new_size, USED, FREE);
        set_header(next_header - WORD_SIZE, new_size, USED, FREE);

        SET_PRE_FREE(next_header); // mark as free on the next block header

    } else {

        uint32_t pre_size = GET_SIZE_AT(header - WORD_SIZE);
        uint32_t next_size = GET_SIZE_AT(next_header);

        uint32_t new_size = pre_size + block_size + next_size;

        char *pre_header = header - pre_size;
        char *next_footer = next_header + next_size - WORD_SIZE;

        set_header(pre_header, new_size, USED, FREE);
        set_header(next_footer, new_size, USED, FREE);

        char *free_next = GET_FREE_NEXT(next_header);

        link_free(pre_header, free_next);
    }

//    printf("\nFREE: %p ", (void *) header);
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

    if (!IS_USED_AT(header)) {
        return NIL;
    }

    if (size == 0) {
        mm_free(block);
        return mm_malloc(size);
    }

    uint32_t old_size = GET_SIZE_AT(header);
    uint32_t new_size = ALIGN(size + WORD_SIZE);

    if (new_size > old_size) {

        char *next_header = header + old_size;

        uint32_t free_size = GET_SIZE_AT(next_header);
        uint32_t missing = new_size - old_size;

        int remain = free_size - missing;

        if (!IS_USED_AT(next_header) && (remain >= 0)) {


            char *free_pre = GET_FREE_PRE(next_header);
            char *free_next = GET_FREE_NEXT(next_header);

            if (remain < MIN_FREE_SIZE) {
                set_header(header, free_size + old_size, IS_PRE_USED(header), USED);

                link_free(free_pre, free_next);

            } else {
                set_header(header, new_size, IS_PRE_USED(header), USED);

                char *new_header = header + new_size;
                char *next_footer = next_header + free_size - WORD_SIZE;

                set_header(new_header, (uint32_t) remain, USED, FREE);
                set_header(next_footer, (uint32_t) remain, USED, FREE);

                link_free(free_pre, new_header);
                link_free(new_header, free_next);
            }

            return block;

        } else {

            char *new_block = (char *) mm_malloc(size);

            memcpy(new_block, block, old_size - WORD_SIZE); // copy payload
            mm_free(block);

            return new_block;
        }
    }

    if (new_size < old_size) {

        uint32_t remain = old_size - new_size;

        if (remain >= MIN_FREE_SIZE) {

            char *old_header = header;
            char *new_header = old_header + new_size;
            char *new_footer = new_header - WORD_SIZE;
            char *old_footer = block + old_size - WORD_SIZE * 2;

            int is_pre_used = IS_PRE_USED(header);
            set_header(old_header, new_size, is_pre_used, USED);
            set_header(new_footer, new_size, is_pre_used, USED);

            // free block and link
            set_header(new_header, remain, USED, FREE);
            set_header(old_footer, remain, USED, FREE);

            add_free(new_header);

        }
    }

    return block;

}
