#ifndef BITMAP_H
#define BITMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BITMAP_MAX_MEM 0x100000000 // 4GB
#define BITMAP_PAGE_SIZE 4096

void bitmap_init(void *start_address, size_t max_memory);

void bitmap_id_map(void);

void *bitmap_alloc_page(void);

void bitmap_free_page(void *addr);

#endif // BITMAP_H
