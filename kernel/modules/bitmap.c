#include "bitmap.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <string.h>
#include "spinlock.h"
#include "../bits.h"
#include "../debug.h"
#include "../cpu/cpuid.h"

static uint32_t *bitmap;
static size_t frames;
static void *start_addr;
static spinlock *bitmap_lock = NULL;

void bitmap_init(void *start_address, size_t max_memory) {
    if (!bitmap_lock) {
        bitmap_lock = spinlock_init();
    }

    start_addr = start_address;
    if ((uintptr_t)start_addr % BITMAP_PAGE_SIZE != 0) { // Round up to the nearest page
        start_addr += BITMAP_PAGE_SIZE - ((uintptr_t)start_addr % BITMAP_PAGE_SIZE);
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

void bitmap_id_map(page_directory_table *pdt) {
    // ID Map the entire bitmap
    mmu_map_pages(pdt, (uintptr_t)bitmap, (uintptr_t)bitmap, frames, true, false, true);
}

void *bitmap_alloc_page(void) {
    spinlock_lock(bitmap_lock);

    size_t bitmap_len = frames / (sizeof(uint32_t) * 8);
    for (size_t i = 0; i < bitmap_len; i++) {
        for (size_t j = 0; j < sizeof(uint32_t) * 8; j++) {
            if (ISSET_BIT(bitmap[i], j)) { // If the page is already allocated
                continue;
            }

            bitmap[i] = ENABLE_BIT(bitmap[i], j);

            spinlock_unlock(bitmap_lock);
            void *addr = start_addr + (i * sizeof(uint32_t) * 8 + j) * BITMAP_PAGE_SIZE;
            dbgprint("Allocated page at 0x%x\n", addr);

            return addr;
        }
    }

    spinlock_unlock(bitmap_lock);
    dbgprint("Failed to allocate page\n");
    return NULL;
}

void *bitmap_alloc_contiguous_pages(size_t pages) {
    spinlock_lock(bitmap_lock);

    size_t bitmap_len = frames / (sizeof(uint32_t) * 8);
    for (size_t i = 0; i < bitmap_len; i++) {
        for (size_t j = 0; j < sizeof(uint32_t) * 8; j++) {
            if (ISSET_BIT(bitmap[i], j)) { // If the page is already allocated
                continue;
            }

            bool found = true;
            size_t ii = i;
            size_t jj = j + 1;
            for (size_t k = 1; k < pages; k++) {
                if (jj == sizeof(uint32_t) * 8) {
                    jj = 0;
                    ii++;
                }

                if (ii == bitmap_len) {
                    found = false;
                    break;
                }

                if (ISSET_BIT(bitmap[ii], jj++)) {
                    found = false;
                    break;
                }
            }

            if (!found) {
                continue;
            }

            ii = i;
            jj = j;
            for (size_t k = 0; k < pages; k++) {
                if (jj == sizeof(uint32_t) * 8) {
                    jj = 0;
                    ii++;
                }

                bitmap[ii] = ENABLE_BIT(bitmap[ii], jj++);
            }

            spinlock_unlock(bitmap_lock);
            void *addr = start_addr + (i * sizeof(uint32_t) * 8 + j) * BITMAP_PAGE_SIZE;
            dbgprint("Allocated %d contiguous pages at 0x%x\n", pages, addr);

            return addr;
        }
    }

    spinlock_unlock(bitmap_lock);
    return NULL;
}

void bitmap_free_pages(void *addr, size_t pages) {
    spinlock_lock(bitmap_lock);

    for (size_t i = 0; i < pages; i++) {
        size_t page_index = (addr - start_addr) / BITMAP_PAGE_SIZE;
        size_t bitmap_index = page_index / (sizeof(uint32_t) * 8);
        size_t bit_index = page_index % (sizeof(uint32_t) * 8);

        bitmap[bitmap_index] = DISABLE_BIT(bitmap[bitmap_index], bit_index);

        dbgprint("Freed page at 0x%x\n", addr);

        addr += BITMAP_PAGE_SIZE;
    }

    spinlock_unlock(bitmap_lock);
}

size_t bitmap_allocated_pages(void) {
    size_t allocated = 0;

    size_t bitmap_len = frames / (sizeof(uint32_t) * 8);
    for (size_t i = 0; i < bitmap_len; i++) {
        if (ISSET_BIT_INT(cpuinfo.ecx, CPUID_FEAT_ECX_POPCNT)) {
            allocated += popcnt64(bitmap[i]);
            continue;
        }

        for (size_t j = 0; j < sizeof(uint32_t) * 8; j++) {
            if (ISSET_BIT(bitmap[i], j)) { // If the page is already allocated
                allocated++;
            }
        }
    }

    return allocated;
}

size_t bitmap_total_pages(void) {
    return frames;
}
