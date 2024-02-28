#include "mmu.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdlib.h>
#include <string.h>
#include "isr.h"
#include "panic.h"
#include "system.h"
#include "../bits.h"
#include "../debug.h"
#include "../modules/bitmap.h"
#include "../modules/spinlock.h"

static uintptr_t kernel_end;
static spinlock *mmu_lock = NULL;
page_directory_table *current_pdt = NULL;

static void page_fault_handler(registers *r, uint32_t int_no) {
    uint32_t cr2;
    asm volatile(
        "mov %%cr2, %0"
        : "=r"(cr2)
    );

    bool present = ISSET_BIT(r->err_code, 0);
    bool write = ISSET_BIT(r->err_code, 1);
    bool user = ISSET_BIT(r->err_code, 2);
    bool reserved = ISSET_BIT(r->err_code, 4);
    bool fetch = ISSET_BIT(r->err_code, 5);

    panic(
        "Page fault at 0x%x, Memory Address: 0x%x\n\n\tTable present?: %s\n\tOperation: %s\n\tRing %d\n\tReserved bits?: %s\n\tFetch?: %s\n",
        r->eip,
        cr2,
        present ? "Y" : "N",
        write ? "Write" : "Read",
        user ? 3 : 0,
        reserved ? "Y" : "N",
        fetch ? "Y" : "N"
    );
}

uintptr_t mmu_get_physical_address(uintptr_t virtual_addr) {
    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;
    uint32_t offset = virtual_addr & 0xFFF;

    page_directory *pd = &current_pdt->entries[pd_index];
    if (!pd->present) {
        return 0;
    }

    page_table *pt = (page_table *) (pd->address << 12);

    page *p = &pt->entries[pt_index];
    if (!p->present) {
        return 0;
    }

    return (p->address << 12) + offset;
}

static void mmu_flush_page(uintptr_t addr) {
    asm volatile("invlpg (%0)" : : "r"(addr) : "memory");
}

page_directory_table *mmu_new_page_directory(void) {
    page_directory_table *pdt = bitmap_alloc_page();
    dbgprint("Page directory table allocated at 0x%x\n", pdt);
    memset(pdt, 0, sizeof(page_directory_table));

    return pdt;
}

void mmu_init(uintptr_t kernel_start_real_addr, uintptr_t kernel_end_real_addr, uintptr_t kernel_start_addr, uintptr_t kernel_end_addr) {
    if (!mmu_lock) {
        mmu_lock = spinlock_init();
    }

    page_directory_table *pdt = mmu_new_page_directory();
    current_pdt = pdt;

    // ID Map the first 4MB of memory
    mmu_map_pages(pdt, 0x0, 0x0, 1024, true, false, true);

    // Map the kernel to 0xc0000000 (higher half)
    mmu_map_pages(pdt, kernel_start_real_addr, 0xc0100000, (kernel_end_real_addr - kernel_start_real_addr) / BITMAP_PAGE_SIZE + 1, true, false, true);

    bitmap_id_map();

    isr_install_handler(14, page_fault_handler);

    kernel_end = kernel_end_addr + 0x1000;
    if (kernel_end % BITMAP_PAGE_SIZE != 0) {
        kernel_end += BITMAP_PAGE_SIZE - (kernel_end % BITMAP_PAGE_SIZE);
    }

    dbgprint("Page directory table initialized\n");
    mmu_enable_paging(current_pdt);
    dbgprint("Paging enabled\n");
}

bool mmu_is_mapped(page_directory_table *pdt, uintptr_t virt_addr) {
    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;

    page_directory *pd = &pdt->entries[pd_index];
    if (!pd->present) {
        return false;
    }

    page_table *pt = (page_table *)(pd->address << 12);

    page *p = &pt->entries[pt_index];
    if (!p->present) {
        return false;
    }

    return true;
}

void mmu_map_pages(page_directory_table *pdt, uintptr_t real_addr, uintptr_t virt_addr, size_t len, bool rw, bool user, bool executable) {
    dbgprint("Mapping %d pages starting from 0x%lx to 0x%lx (rw=%c, user=%c, exec=%c)\n", len, real_addr, virt_addr, rw ? 'Y' : 'N', user ? 'Y' : 'N', executable ? 'Y' : 'N');

    spinlock_lock(mmu_lock);

    for (size_t i = 0; i < len; i++) {
        uint32_t pd_index = virt_addr >> 22;
        uint32_t pt_index = (virt_addr >> 12) & 0x3FF;

        page_directory *pd = &pdt->entries[pd_index];
        page_table *pt = NULL;
        if (pd->present) {
            pt = (page_table *)(pd->address << 12);
        } else {
            dbgprint("Creating new page table\n");
            pt = bitmap_alloc_page();
            memset(pt, 0, sizeof(page_table));

            pd->present = 1;
        }

        pd->rw = rw;
        pd->user = user;
        pd->address = (uint32_t)pt >> 12;

        page *p = &pt->entries[pt_index];
        p->present = 1;
        p->rw = rw;
        p->user = user;
        p->address = real_addr >> 12;

        mmu_flush_page(virt_addr);

        real_addr += BITMAP_PAGE_SIZE;
        virt_addr += BITMAP_PAGE_SIZE;
    }

    spinlock_unlock(mmu_lock);
}

void mmu_unmap_pages(page_directory_table *pdt, uintptr_t virt_addr, size_t len) {
    dbgprint("Unmapping %d pages starting from 0x%lx\n", len, virt_addr);

    spinlock_lock(mmu_lock);

    for (size_t i = 0; i < len; i++) {
        uint32_t pd_index = virt_addr >> 22;
        uint32_t pt_index = (virt_addr >> 12) & 0x3FF;

        page_directory *pd = &pdt->entries[pd_index];
        if (pd->present) {
            page_table *pt = (page_table *)(pd->address << 12);

            page *p = &pt->entries[pt_index];
            p->present = 0;
            p->rw = 0;
            p->user = 0;
            p->address = 0;

            mmu_flush_page(virt_addr);
        }

        virt_addr += BITMAP_PAGE_SIZE;
    }

    spinlock_unlock(mmu_lock);
}

// Alloc `pages` pages of kernel memory in a contiguous block and return a pointer to the first page
void *mmu_alloc_pages(size_t pages) {
    spinlock_lock(mmu_lock);

    void *start_addr = NULL;
    for (uintptr_t addr = kernel_end; addr < 0xFFFFFFFF; addr += BITMAP_PAGE_SIZE) {
        // Search for `pages` pages of contiguous memory
        size_t i;
        for (i = 0; i < pages; i++) {
            if (mmu_is_mapped(current_pdt, addr + i * BITMAP_PAGE_SIZE)) {
                break;
            }
        }

        if (i == pages) {
            start_addr = (void *)addr;
            break;
        }
    }

    spinlock_unlock(mmu_lock);

    if (!start_addr) {
        dbgprint("Failed to allocate %d pages\n", pages);
        return NULL;
    }

    for (size_t i = 0; i < pages; i++) {
        uintptr_t virt_addr = (uintptr_t)start_addr + i * BITMAP_PAGE_SIZE;
        uintptr_t real_addr = (uintptr_t)bitmap_alloc_page();

        mmu_map_pages(current_pdt, real_addr, virt_addr, 1, true, false, false);
    }

    return start_addr;
}

void mmu_free_pages(void *addr, size_t pages) {
    for (size_t i = 0; i < pages; i++) {
        uintptr_t virt_addr = (uintptr_t)addr + i * BITMAP_PAGE_SIZE;
        uintptr_t real_addr = mmu_get_physical_address(virt_addr);

        if (!mmu_is_mapped(current_pdt, virt_addr)) {
            continue;
        }

        bitmap_free_page((void *)real_addr);

        mmu_unmap_pages(current_pdt, virt_addr, 1);
    }
}
