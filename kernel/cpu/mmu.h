#ifndef KERNEL_MMU_H
#define KERNEL_MMU_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define MMU_MAX_PAGES 1024 * 1024

#pragma pack(push, 1)
typedef struct page_directory {
    bool present: 1;
    bool rw: 1;
    bool user: 1;
    bool write_through: 1;
    bool cache_disabled: 1;
    bool accessed: 1;
    bool dirty: 1;
    bool page_size: 1;
    uint8_t available: 4;
    uint32_t address: 20; // -> page_table
} page_directory;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct page {
    bool present: 1;
    bool rw: 1;
    bool user: 1;
    bool write_through: 1;
    bool cache_disabled: 1;
    bool accessed: 1;
    bool dirty: 1;
    bool pat: 1;
    bool global: 1;
    uint8_t available: 3;
    uint32_t address: 20; // -> physical address
} page;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct page_directory_table {
    page_directory entries[1024];
} page_directory_table;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct page_table {
    page entries[1024];
} page_table;
#pragma pack(pop)

extern page_directory_table *current_pdt;

extern void mmu_enable_paging(page_directory_table *pdt);

void mmu_load_pdt(page_directory_table *pdt);

uintptr_t mmu_get_physical_address(uintptr_t virtual_addr);

uintptr_t mmu_get_physical_address_pdt(uintptr_t virtual_addr, page_directory_table *pdt);

page_directory_table *mmu_new_page_directory(void);

void mmu_init(uintptr_t kernel_start_real_addr, uintptr_t kernel_end_real_addr, uintptr_t kernel_start_addr, uintptr_t kernel_end_addr);

bool mmu_is_mapped(page_directory_table *pdt, uintptr_t virt_addr);

void mmu_map_pages(page_directory_table *pdt, uintptr_t real_addr, uintptr_t virt_addr, size_t len, bool rw, bool user, bool executable);

void mmu_unmap_pages(page_directory_table *pdt, uintptr_t virt_addr, size_t len);

void *mmu_alloc_pages(size_t pages);

void mmu_free_pages(page_directory_table *pdt, void *addr, size_t pages);

void mmu_copy_kernel_pages(page_directory_table *pdt);

page_directory_table *mmu_clone_pdt(page_directory_table *pdt);

void mmu_load_kernel_pdt(void);

#endif // KERNEL_MMU_H
