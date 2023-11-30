#include "bitmap.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <string.h>
#include "spinlock.h"
#include "../bits.h"
#include "../debug.h"
#include "../cpu/mmu.h"

//static uint32_t bitmap[BITMAP_MAX_MEM / BITMAP_PAGE_SIZE / (sizeof(uint32_t) * 8)] = {0};
static uint32_t *bitmap;
static size_t frames;
static void *start_addr;
static spinlock *lock = NULL;

void bitmap_init(void *start_address, size_t max_memory) {
    if (!lock) {
        lock = spinlock_init();
    }

    start_addr = start_address;
    if ((uint32_t)start_addr % BITMAP_PAGE_SIZE != 0) { // Round up to the nearest page
        start_addr += BITMAP_PAGE_SIZE - ((uint32_t)start_addr % BITMAP_PAGE_SIZE);
    }

    size_t max_mem = max_memory - (size_t)start_addr;
    if (max_mem % BITMAP_PAGE_SIZE != 0) { // Round up to the nearest page
        max_mem += BITMAP_PAGE_SIZE - (max_mem % BITMAP_PAGE_SIZE);
    }

    // Allocate the bitmap in the first 32 pages
    // Each page can store 4096 bytes * 8 = 32768 bits
    // If each bit represents a page, each page can represent 32768 pages = 128MB
    // 32 pages * 128MB = 4GB
    bitmap = start_addr;
    memset(bitmap, 0, (frames = max_mem / BITMAP_PAGE_SIZE) / 8);
    bitmap[0] = 0xFFFFFFFF;

    dbgprint("Bitmap initialized with %d pages starting from &%x\n", frames, start_addr);
}

void bitmap_id_map(void) {
    // ID Map the entire bitmap
    mmu_map_pages(current_pdt, (uintptr_t)bitmap, (uintptr_t)bitmap, frames, true, false, true);
}

void *bitmap_alloc_page(void) {
    spinlock_lock(lock);

    size_t bitmap_len = frames / (sizeof(uint32_t) * 8);
    for (size_t i = 0; i < bitmap_len; i++) {
        for (size_t j = 0; j < sizeof(uint32_t) * 8; j++) {
            if (ISSET_BIT(bitmap[i], j)) { // If the page is already allocated
                continue;
            }

            bitmap[i] = ENABLE_BIT(bitmap[i], j);

            spinlock_unlock(lock);
            void *addr = start_addr + (i * sizeof(uint32_t) * 8 + j) * BITMAP_PAGE_SIZE;
            dbgprint("Allocated page at 0x%x\n", addr);

            return addr;
        }
    }

    spinlock_unlock(lock);
    dbgprint("Failed to allocate page\n");
    return NULL;
}

void bitmap_free_page(void *addr) {
    spinlock_lock(lock);

    size_t page_index = (addr - start_addr) / BITMAP_PAGE_SIZE;
    size_t bitmap_index = page_index / sizeof(uint32_t) * 8;
    size_t bit_index = page_index % sizeof(uint32_t) * 8;

    bitmap[bitmap_index] = DISABLE_BIT(bitmap[bitmap_index], bit_index);

    spinlock_unlock(lock);
    dbgprint("Freed page at 0x%x\n", addr);
}
