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
static page_directory_table *kernel_pdt = NULL;
page_directory_table *current_pdt = NULL;

void mmu_load_pdt(page_directory_table *pdt) {
    asm volatile(
        "mov %%cr3, %0"
        : "=r"(current_pdt)
    );

    if (pdt == current_pdt) {
        return;
    }

    asm volatile(
        "mov %0, %%cr3"
        :
        : "r"(pdt)
    );
}

static void page_fault_handler(registers *r, uint32_t int_no) {
    uint32_t cr2;
    asm volatile(
        "mov %%cr2, %0"
        : "=r"(cr2)
    );

    bool present    = ISSET_BIT(r->err_code, 0);
    bool write      = ISSET_BIT(r->err_code, 1);
    bool user       = ISSET_BIT(r->err_code, 2);
    bool reserved   = ISSET_BIT(r->err_code, 4);
    bool fetch      = ISSET_BIT(r->err_code, 5);
    bool protection = ISSET_BIT(r->err_code, 6);
    bool shadow     = ISSET_BIT(r->err_code, 7);
    bool sgx        = ISSET_BIT(r->err_code, 15);

    // TODO: Handle page faults properly
    panic_handler(
        r,
        "Page fault at 0x%x, Memory Address: 0x%x - %s%s%s%s%s%s%s%s",
        r->eip,
        cr2,
        present ? "P" : "",
        write ? "W" : "R",
        user ? "U" : "K",
        reserved ? "R" : "",
        fetch ? "F" : "",
        protection ? " PK" : "",
        shadow ? " SS" : "",
        sgx ? " SGX" : ""
    );
}

uintptr_t mmu_get_physical_address(uintptr_t virtual_addr) {
    return mmu_get_physical_address_pdt(virtual_addr, current_pdt);
}

uintptr_t mmu_get_physical_address_pdt(uintptr_t virtual_addr, page_directory_table *pdt) {
    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;
    uint32_t offset = virtual_addr & 0xFFF;

    page_directory *pd = &pdt->entries[pd_index];
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
    kernel_pdt = pdt;
    current_pdt = pdt;

    // ID Map the first 4MB of memory
    mmu_map_pages(pdt, 0x0, 0x0, 1024, true, false, true);

    // Map the kernel to 0xc0000000 (higher half)
    mmu_map_pages(pdt, kernel_start_real_addr, 0xc0100000, (kernel_end_real_addr - kernel_start_real_addr) / BITMAP_PAGE_SIZE + 1, true, false, true);

    // ID Map the bitmap
    bitmap_id_map(pdt);

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

        if (pdt == current_pdt) {
            mmu_flush_page(virt_addr);
        }

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

            if (pdt == current_pdt) {
                mmu_flush_page(virt_addr);
            }
        }

        virt_addr += BITMAP_PAGE_SIZE;
    }

    spinlock_unlock(mmu_lock);
}

void *mmu_map_large_pages(page_directory_table *pdt, uintptr_t real_addr, uintptr_t virt_addr, size_t len, bool rw, bool user, bool executable) {
    dbgprint("Mapping %d large pages starting from 0x%lx to 0x%lx (rw=%c, user=%c, exec=%c)\n", len, real_addr, virt_addr, rw ? 'Y' : 'N', user ? 'Y' : 'N', executable ? 'Y' : 'N');

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
        //pd->page_size = 1;

        page *p = &pt->entries[pt_index];
        p->present = 1;
        p->rw = rw;
        p->user = user;
        p->address = real_addr >> 22;

        if (pdt == current_pdt) {
            mmu_flush_page(virt_addr);
        }

        real_addr += BITMAP_PAGE_SIZE * 1024;
        virt_addr += BITMAP_PAGE_SIZE * 1024;
    }

    spinlock_unlock(mmu_lock);

    return (void *)virt_addr;
}

// Alloc n pages of kernel memory in a contiguous block and return a pointer to the first page
void *mmu_alloc_pages(size_t pages) {
    spinlock_lock(mmu_lock);

    void *start_addr = NULL;
    for (uintptr_t addr = kernel_end; addr < 0xFFFFFFFF; addr += BITMAP_PAGE_SIZE) {
        // Search for n pages of contiguous memory
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

    uintptr_t real_addr = (uintptr_t)bitmap_alloc_contiguous_pages(pages);
    if (!real_addr) {
        dbgprint("Failed to allocate %d pages\n", pages);
        return NULL;
    }

    mmu_map_pages(current_pdt, real_addr, (uintptr_t)start_addr, pages, true, false, false);

    return start_addr;
}

void mmu_free_pages(page_directory_table *pdt, void *addr, size_t pages) {
    for (size_t i = 0; i < pages; i++) {
        uintptr_t virt_addr = (uintptr_t)addr + i * BITMAP_PAGE_SIZE;
        uintptr_t real_addr = mmu_get_physical_address_pdt(virt_addr, pdt);

        if (!mmu_is_mapped(pdt, virt_addr)) {
            continue;
        }

        bitmap_free_pages((void *)real_addr, 1);

        mmu_unmap_pages(pdt, virt_addr, 1);
    }
}

void mmu_copy_kernel_pages(page_directory_table *pdt) {
    // Kernel pages are already mapped in the higher half, so we just need to copy the mappings
    for (uintptr_t pd_index = 0xc0000000 >> 22; pd_index <= 0xFFFFFFFF >> 22; pd_index++) {
        dbgprint("Copying page directory entry %d\n", pd_index);

        page_directory *pd = &kernel_pdt->entries[pd_index];

        memcpy(&pdt->entries[pd_index], pd, sizeof(page_directory));
    }
}

page_directory_table *mmu_clone_pdt(page_directory_table *pdt) {
    page_directory_table *new_pdt = mmu_new_page_directory();
    if (!new_pdt) {
        return NULL;
    }

    for (size_t i = 0; i < 0xc0000000 >> 22; i++) {
        page_directory *pd = &pdt->entries[i];
        page_directory *new_pd = &new_pdt->entries[i];

        if (pd->present) {
            page_table *pt = (page_table *)(pd->address << 12);
            page_table *new_pt = bitmap_alloc_page();
            if (!new_pt) {
                return NULL;
            }

            memset(new_pt, 0, sizeof(page_table));

            memcpy(new_pd, pd, sizeof(page_directory));
            new_pd->address = (uint32_t)new_pt >> 12;

            for (size_t j = 0; j < 1024; j++) {
                page *p = &pt->entries[j];
                page *new_p = &new_pt->entries[j];

                if (p->present) {
                    void *new_page = bitmap_alloc_page();
                    if (!new_page) {
                        return NULL;
                    }

                    memcpy(new_p, p, sizeof(page));
                    new_p->address = (uint32_t)new_page >> 12;

                    memcpy((void *)(new_p->address << 12), (void *)(p->address << 12), BITMAP_PAGE_SIZE);
                }
            }
        }
    }

    mmu_copy_kernel_pages(new_pdt);

    return new_pdt;
}

static page_directory_table *old_pdt;
void mmu_load_kernel_pdt(void) {
    if (kernel_pdt) {
        mmu_load_pdt(kernel_pdt);
        if (current_pdt != kernel_pdt) {
            dbgprint("Loading kernel page directory table %x -> %x\n", current_pdt, kernel_pdt);
        }

        old_pdt = current_pdt;
        current_pdt = kernel_pdt;
    }
}

void mmu_restore_pdt(void) {
    if (old_pdt) {
        mmu_load_pdt(old_pdt);
        if (current_pdt != old_pdt) {
            dbgprint("Restoring page directory table %x -> %x\n", current_pdt, old_pdt);
        }

        current_pdt = old_pdt;
        old_pdt = NULL;
    }
}

void mmu_writeb(page_directory_table *pdt, uintptr_t virt_addr, uint8_t data) {
    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
    uint32_t offset = virt_addr & 0xFFF;

    page_directory *pd = &pdt->entries[pd_index];
    if (!pd->present) {
        return;
    }

    page_table *pt = (page_table *)(pd->address << 12);

    page *p = &pt->entries[pt_index];
    if (!p->present) {
        return;
    }

    uint8_t *real_addr = (uint8_t *)((p->address << 12) + offset);
    *real_addr = data;
}

void mmu_writew(page_directory_table *pdt, uintptr_t virt_addr, uint16_t data) {
    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
    uint32_t offset = virt_addr & 0xFFF;

    page_directory *pd = &pdt->entries[pd_index];
    if (!pd->present) {
        return;
    }

    page_table *pt = (page_table *)(pd->address << 12);

    page *p = &pt->entries[pt_index];
    if (!p->present) {
        return;
    }

    uint16_t *real_addr = (uint16_t *)((p->address << 12) + offset);
    *real_addr = data;
}

void mmu_writel(page_directory_table *pdt, uintptr_t virt_addr, uint32_t data) {
    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
    uint32_t offset = virt_addr & 0xFFF;

    page_directory *pd = &pdt->entries[pd_index];
    if (!pd->present) {
        return;
    }

    page_table *pt = (page_table *)(pd->address << 12);

    page *p = &pt->entries[pt_index];
    if (!p->present) {
        return;
    }

    uint32_t *real_addr = (uint32_t *)((p->address << 12) + offset);
    *real_addr = data;
}

uint8_t mmu_readb(page_directory_table *pdt, uintptr_t virt_addr) {
    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
    uint32_t offset = virt_addr & 0xFFF;

    page_directory *pd = &pdt->entries[pd_index];
    if (!pd->present) {
        return 0;
    }

    page_table *pt = (page_table *)(pd->address << 12);

    page *p = &pt->entries[pt_index];
    if (!p->present) {
        return 0;
    }

    uint8_t *real_addr = (uint8_t *)((p->address << 12) + offset);
    return *real_addr;
}

uint16_t mmu_readw(page_directory_table *pdt, uintptr_t virt_addr) {
    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
    uint32_t offset = virt_addr & 0xFFF;

    page_directory *pd = &pdt->entries[pd_index];
    if (!pd->present) {
        return 0;
    }

    page_table *pt = (page_table *)(pd->address << 12);

    page *p = &pt->entries[pt_index];
    if (!p->present) {
        return 0;
    }

    uint16_t *real_addr = (uint16_t *)((p->address << 12) + offset);
    return *real_addr;
}

uint32_t mmu_readl(page_directory_table *pdt, uintptr_t virt_addr) {
    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
    uint32_t offset = virt_addr & 0xFFF;

    page_directory *pd = &pdt->entries[pd_index];
    if (!pd->present) {
        return 0;
    }

    page_table *pt = (page_table *)(pd->address << 12);

    page *p = &pt->entries[pt_index];
    if (!p->present) {
        return 0;
    }

    uint32_t *real_addr = (uint32_t *)((p->address << 12) + offset);
    return *real_addr;
}

size_t mmu_strlen(page_directory_table *pdt, uintptr_t virt_addr) {
    size_t len = 0;
    while (mmu_readb(pdt, virt_addr + len) != '\0') {
        len++;
    }
    return len;
}

void *mmu_memcpy(page_directory_table *pdt, uintptr_t virt_addr, void *destination, size_t n) {
    size_t i = 0;

    for (i = 0; i < n / 4; i++) {
        *(uint32_t *)destination = mmu_readl(pdt, virt_addr);

        virt_addr += 4;
        destination += 4;
    }

    n -= i * 4;

    for (i = 0; i < n / 2; i++) {
        *(uint16_t *)destination = mmu_readw(pdt, virt_addr);

        virt_addr += 2;
        destination += 2;
    }

    n -= i * 2;

    uint8_t *c_dest = (uint8_t *)destination;
    while (n--) {
        *c_dest++ = mmu_readb(pdt, virt_addr);
        virt_addr++;
    }
}

char *mmu_strcpy(page_directory_table *pdt, uintptr_t virt_addr, char *destination) {
    size_t len = mmu_strlen(pdt, virt_addr);
    mmu_memcpy(pdt, virt_addr, destination, len);
    destination[len] = 0;

    return destination;
}

char *mmu_strncpy(page_directory_table *pdt, uintptr_t virt_addr, char *destination, size_t n) {
    size_t len = mmu_strlen(pdt, virt_addr);
    mmu_memcpy(pdt, virt_addr, destination, len < n ? len : n);

    for (size_t i = len; i < n; i++) {
        destination[i] = 0;
    }

    destination[n] = 0;

    return destination;
}
