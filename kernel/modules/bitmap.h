#ifndef BITMAP_H
#define BITMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../cpu/mmu.h"

#define BITMAP_MAX_MEM 0x100000000 // 4GB
#define BITMAP_PAGE_SIZE 4096

void bitmap_init(void *start_address, size_t max_memory);

void bitmap_id_map(page_directory_table *pdt);

void *bitmap_alloc_page(void);

void *bitmap_alloc_contiguous_pages(size_t pages);

void bitmap_free_pages(void *addr, size_t pages);

size_t bitmap_allocated_pages(void);

size_t bitmap_total_pages(void);

#endif // BITMAP_H
