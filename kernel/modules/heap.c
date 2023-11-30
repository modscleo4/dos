#include "heap.h"

#define DEBUG 0
#define DEBUG_SERIAL 1

#include "../debug.h"

void heap_init(heap *this) {
    this->first_block = NULL;
}

int heap_add_block(heap *this, void *addr, size_t size, size_t block_size) {
    heap_block *hb;
    uint32_t block_count;
    uint8_t *bitmap;

    hb = (heap_block *)addr;
    hb->size = size - sizeof(heap_block);
    hb->block_size = block_size;

    hb->next = this->first_block;
    this->first_block = hb;

    block_count = hb->size / hb->block_size;
    bitmap = (uint8_t *)&hb[1];

    /* clear bitmap */
    for (size_t x = 0; x < block_count; x++) {
        bitmap[x] = 0;
    }

    /* reserve room for bitmap */
    block_count = (block_count / block_size) * block_size < block_count ? block_count / block_size + 1 : block_count / block_size;
    for (size_t x = 0; x < block_count; x++) {
        bitmap[x] = 5;
    }

    hb->lfb = block_count - 1;

    hb->used = block_count;

    return 1;
}

static uint8_t heap_get_new_id(uint8_t a, uint8_t b) {
    uint8_t c;
    for (c = a + 1; c == b || c == 0; c++) {}
    return c;
}

void *heap_alloc(heap *this, size_t size) {
    heap_block *hb;
    uint8_t *bitmap;
    uint32_t block_count;
    uint32_t x, y, z;
    uint32_t blocks_needed;
    uint8_t new_id;

    /* iterate blocks */
    for (hb = this->first_block; hb; hb = hb->next) {
        /* check if block has enough room */
        if (hb->size - (hb->used * hb->block_size) >= size) {
            block_count = hb->size / hb->block_size;
            blocks_needed = (size / hb->block_size) * hb->block_size < size ? size / hb->block_size + 1 : size / hb->block_size;
            bitmap = (uint8_t *)&hb[1];
            dbgprint("Heap block has enough room\n");
            dbgprint("  hb->size: %d, hb->block_size: %d\n", hb->size, hb->block_size);
            dbgprint("  block_count: %d\n", block_count);
            dbgprint("  blocks_needed: %d\n", blocks_needed);
            dbgprint("  x: %d, lfb = %d\n", (hb->lfb + 1 >= block_count ? 0 : hb->lfb + 1), hb->lfb);

            for (x = (hb->lfb + 1 >= block_count ? 0 : hb->lfb + 1); x != hb->lfb; x++) {
                /* just wrap around */
                if (x >= block_count) {
                    x = 0;
                }

                dbgprint("  bitmap[%d]: %d\n", x, bitmap[x]);

                if (bitmap[x] == 0) {
                    /* count free blocks */
                    for (y = 0; bitmap[x + y] == 0 && y < blocks_needed && (x + y) < block_count; ++y) {}

                    /* we have enough, now allocate them */
                    if (y == blocks_needed) {
                        /* find ID that does not match left or right */
                        new_id = heap_get_new_id(bitmap[x - 1], bitmap[x + y]);

                        /* allocate by setting id */
                        for (z = 0; z < y; ++z) {
                            bitmap[x + z] = new_id;
                        }

                        /* optimization */
                        hb->lfb = (x + blocks_needed) - 2;

                        /* count used blocks NOT bytes */
                        hb->used += y;

                        return (void *)(x * hb->block_size + (uintptr_t)&hb[1]);
                    }

                    /* x will be incremented by one ONCE more in our FOR loop */
                    x += (y - 1);
                    continue;
                }
            }
        } else {
            dbgprint("Heap block does not have enough room\n");
        }
    }

    dbgprint("Heap block not found\n");
    return NULL;
}

bool heap_free(heap *this, void *addr) {
    heap_block *hb;
    uintptr_t pointer_offset;
    uint32_t bi, x;
    uint8_t *bitmap;
    uint8_t id;
    uint32_t max;

    for (hb = this->first_block; hb; hb = hb->next) {
        if ((uintptr_t)addr > (uintptr_t)hb && (uintptr_t)addr < (uintptr_t)hb + sizeof(heap_block) + hb->size) {
            /* found block */
            pointer_offset = (uintptr_t)addr - (uintptr_t)&hb[1]; /* get offset to get block */
            /* block offset in BM */
            bi = pointer_offset / hb->block_size;
            /* .. */
            bitmap = (uint8_t *)&hb[1];
            /* clear allocation */
            id = bitmap[bi];
            /* oddly.. GCC did not optimize this */
            max = hb->size / hb->block_size;
            for (x = bi; bitmap[x] == id && x < max; ++x) {
                bitmap[x] = 0;
            }
            /* update free block count */
            hb->used -= x - bi;
            return true;
        }
    }

    /* this error needs to be raised or reported somehow */
    return false;
}
