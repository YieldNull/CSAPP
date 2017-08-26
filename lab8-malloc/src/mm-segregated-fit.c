/**
 * mm.c
 *
 *  Use Segregated Fits
 *
 * 1   2   3-4   5-8   9-16   17-32
 * --------------------------------
 * 0   1    2     3     4       5
 *
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

#define MIN_FREE_SIZE ALIGN(WORD_SIZE * 4)

#define LIST_SZIE (32)

/* mask to calculate block size */
#define SIZE_MASK (~0x7)

/* get word at address */
#define GET_WORD_AT(p) (*((uint32_t *) (p)))

/* set word at address */
#define SET_FREE_AT(p, value) (*((uint32_t *)(p)) = (value))

/* get size of block from header or footer at address */
#define GET_SIZE_AT(p) (GET_WORD_AT(p) & SIZE_MASK)

/* set size of block to header or footer at address
 * and mark block as used.
 */
#define SET_USED_AT(p, size) (SET_FREE_AT(p, ((size) | 0x1)))

/*  check the flag bit of a header or footer at the address
 *  to see whether the block is used
 */
#define IS_USED_AT(p) ((GET_WORD_AT(p) & 0x1) == 1)

#define GET_FREE_PRE(p) ((char *)(*(uint32_t *)((p) + WORD_SIZE)))

#define GET_FREE_NEXT(p) ((char *)(*(uint32_t *)((p) + WORD_SIZE + PTR_SIZE)))

#define SET_FREE_PRE(p, ptr) (*((uint32_t *) ((p) + WORD_SIZE)) = (uint32_t)(ptr))

#define SET_FREE_NEXT(p, ptr) (*((uint32_t *) ((p) + WORD_SIZE + PTR_SIZE)) = (uint32_t)(ptr))

static char *heap_start; // start address of heap, after prologue

static char *heap_ceil; // ceil address of heap, the address of epilogue

static char *lists[LIST_SZIE];

/*
 * ilog2_floor - return floor(log base 2 of x), where x > 0
 *   Example: ilog2_floor(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2_floor(int x) {

    int tmp, mask1, mask2, mask3, mask4, mask5, v;

    // most significant bit
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);  // eg. 0010 1011 => 0011 1111
    // MSB is x & (~(x >> 1))

    // bitCount
    tmp = 0x55 | (0x55 << 8);
    mask1 = tmp | (tmp << 16); // 0x55555555

    tmp = 0x33 | (0x33 << 8);
    mask2 = tmp | (tmp << 16); // 0x33333333

    tmp = 0x0F | (0x0F << 8);
    mask3 = tmp | (tmp << 16); // 0x0F0F0F0F

    mask4 = 0xFF | (0xFF << 16); // 0x00FF00FF

    tmp = 0xFF | (0xFF << 8);
    mask5 = tmp | (0 << 16); // 0x0000FFFF

    v = (x & mask1) + ((x >> 1) & mask1);
    v = (v & mask2) + ((v >> 2) & mask2);
    v = (v & mask3) + ((v >> 4) & mask3);
    v = (v & mask4) + ((v >> 8) & mask4);
    v = (v & mask5) + ((v >> 16) & mask5);

    return v - 1;
}


int ilog2_ceil(uint32_t base) {
    if ((base & (base - 1)) == 0) { // 2^n
        return ilog2_floor(base);
    } else {
        return ilog2_floor(base) + 1;
    }
}

/**
 * 1   2   3-4   5-8   9-16   17-32
 * --------------------------------
 * 0   1    2     3     4       5
 * */
int index_in_lists(uint32_t block_size) {
    uint32_t base = ((block_size - WORD_SIZE * 2) / ALIGNMENT); // index using payload size
    return ilog2_ceil(base);
}

void link_free(char *pre, char *next) {
    if (pre != NULL) {
        SET_FREE_NEXT(pre, next);
    }

    if (next != NULL) {
        SET_FREE_PRE(next, pre);
    }
}

/**
 * Add a free block at the front of a list
 * */
void add_free(char *free, uint32_t block_size) {
    int index = index_in_lists(block_size);

    SET_FREE_PRE(free, NULL);
    SET_FREE_NEXT(free, NULL);

    // insert at front
    char *header = lists[index];
    link_free(free, header);

    lists[index] = free;
}

void remove_free(char *free) {
    char *pre = GET_FREE_PRE(free);
    char *next = GET_FREE_NEXT(free);

    link_free(pre, next);

    if (pre == NULL) {
        int index = index_in_lists(GET_SIZE_AT(free));
        lists[index] = next;
    }
}

/**
 * Find a free block fitted to the block size
 * */
char *find_free(uint32_t block_size) {
    int index = index_in_lists(block_size);

    for (int i = index; i < LIST_SZIE; ++i) {
        char *ptr = lists[i];

        while (ptr != NULL) {
            if (GET_SIZE_AT(ptr) >= block_size) {
                return ptr;
            }

            ptr = GET_FREE_NEXT(ptr);
        }
    }

    return NULL;
}


void truncate_block(char *header, uint32_t required_size, int is_free) {
    if (is_free) {
        remove_free(header);
    }

    uint32_t free_size = GET_SIZE_AT(header);
    char *footer = header + free_size - WORD_SIZE;

    int remain_size = free_size - required_size;

    if (remain_size >= MIN_FREE_SIZE) {
        char *new_header = header + required_size;

        SET_USED_AT(header, required_size);
        SET_USED_AT(new_header - WORD_SIZE, required_size);


        SET_FREE_AT(new_header, remain_size);
        SET_FREE_AT(footer, remain_size);

        add_free(new_header, (uint32_t) remain_size);

    } else {
        SET_USED_AT(header, free_size);
        SET_USED_AT(footer, free_size);
    }
}

char *alloc_request(uint32_t alloc_size) {
    void *ptr = mem_sbrk(alloc_size);

    if (ptr == (void *) -1) {
        return NULL;
    }

    char *header = heap_ceil;
    heap_ceil = header + alloc_size;

    SET_USED_AT(header, alloc_size);
    SET_USED_AT(heap_ceil - WORD_SIZE, alloc_size);

    SET_USED_AT(heap_ceil, 0);

    return header;
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
    SET_USED_AT(heap_start, 0);

    /* epilogue */
    SET_USED_AT(heap_ceil - WORD_SIZE, 0);

    heap_start = heap_start + WORD_SIZE;
    heap_ceil = heap_ceil - WORD_SIZE;


    for (int i = 0; i < LIST_SZIE; ++i) {
        lists[i] = NULL;
    }

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {

    if (size == 0) {
        return NULL;
    }

    uint32_t block_size = ALIGN(size + WORD_SIZE * 2);

    char *header = find_free(block_size);

    if (header == NULL) { // use mem_sbrk

        int pow = ilog2_ceil(ALIGN(size));
        uint32_t alloc_size = (uint32_t) (1 << pow) + WORD_SIZE * 2;

        header = alloc_request(alloc_size);

        if (header == NULL) {
            return NULL;
        }

        truncate_block(header, block_size, 0);

        return (void *) (header + WORD_SIZE);
    }

    truncate_block(header, block_size, 1);

    return (void *) (header + WORD_SIZE);

}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *block) {
    char *header, *header_next, *header_pre, *footer, *footer_next, *footer_pre;
    uint32_t block_size, pre_size, next_size;

    char *new_header, *new_footer;
    uint32_t new_size;

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

        new_size = block_size;
        new_header = header;
        new_footer = footer;

    } else if (IS_USED_AT(footer_pre) && !IS_USED_AT(header_next)) {

        new_size = block_size + next_size;
        new_header = header;
        new_footer = footer_next;

        remove_free(header_next);

    } else if (!IS_USED_AT(footer_pre) && IS_USED_AT(header_next)) {

        new_size = block_size + pre_size;
        new_header = header_pre;
        new_footer = footer;

        remove_free(header_pre);

    } else {

        new_size = block_size + pre_size + next_size;
        new_header = header_pre;
        new_footer = footer_next;

        remove_free(header_pre);
        remove_free(header_next);
    }

    SET_FREE_AT(new_header, new_size);
    SET_FREE_AT(new_footer, new_size);

    add_free(new_header, new_size);
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
        return NULL;
    }

    if (size == 0) {
        mm_free(block);
        return mm_malloc(size);
    }

    if (new_size > old_size) {
        int missing = new_size - old_size;
        char *next_block = block + old_size;

        char *next_header = next_block - WORD_SIZE;
        uint32_t next_size = GET_SIZE_AT(next_header);
        int remain_size = next_size - missing;

        if (!IS_USED_AT(next_header) && (remain_size >= 0)) {

            remove_free(next_header);

            char *next_footer = next_block + next_size - WORD_SIZE * 2;

            if (remain_size >= MIN_FREE_SIZE) {
                char *new_footer = block + new_size - WORD_SIZE * 2;
                char *new_header = new_footer + WORD_SIZE;

                SET_USED_AT(header, new_size);
                SET_USED_AT(new_footer, new_size);

                add_free(new_header, (uint32_t) remain_size);
                SET_FREE_AT(new_header, remain_size);
                SET_FREE_AT(next_footer, remain_size);

            } else {
                SET_USED_AT(header, old_size + next_size);
                SET_USED_AT(next_footer, old_size + next_size);
            }

            return block;

        } else if (next_header == heap_ceil) {
            char *allocated = alloc_request((uint32_t) missing);

            SET_USED_AT(header, new_size);
            SET_USED_AT(allocated + missing - WORD_SIZE, new_size);

            return block;
        } else {

            char *new_block = (char *) mm_malloc(size);

            memcpy(new_block, block, old_size - WORD_SIZE * 2); // copy payload
            mm_free(block);

            return new_block;
        }
    } else if (new_size < old_size) {

        truncate_block(header, new_size, 0);

        return block;
    } else {
        return block;
    }
}
