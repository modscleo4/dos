#ifndef HEAP_H
#define HEAP_H

/**
 * Bitmap Heap
 */

#include <stddef.h>
#include <stdint.h>

typedef struct heap_block {
    struct heap_block *next;
    size_t size;
    size_t used;
    size_t block_size;
    size_t lfb;
} heap_block;

typedef struct heap {
    heap_block *first_block;
} heap;

void heap_init(heap *this);

int heap_add_block(heap *this, void *addr, size_t size, size_t block_size);

void *heap_alloc(heap *this, size_t size);

void heap_free(heap *this, void *addr);

#endif // HEAP_H
