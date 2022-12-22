#include "mmu.h"

#include <string.h>
#include "isr.h"
#include "system.h"

page_directory_table *current_pd;

static void page_fault_handler(registers *r, uint32_t int_no) {

}

static uint32_t mmu_get_physical_address(uint32_t virtual_addr) {
    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;
    uint32_t offset = virtual_addr & 0xFFF;

    page_directory *pd = &current_pd->entries[pd_index];
    page_table *pt = (page_table *) (pd->address << 12);

    page *p = &pt->entries[pt_index];

    return (p->address << 12) + offset;
}

static page_directory *mmu_get_page_directory(uint32_t addr) {
    //
}

void mmu_init(void) {
    /*page_directory_table pdt;
    memset(&pdt, 0, sizeof(page_directory_table));

    for (int i = 0; i < 1024; i++) {
        pdt.entries[i].present = 0;
        pdt.entries[i].rw = 1;
        pdt.entries[i].user = 0;
    }

    page_table pt;
    page_table kpt;
    memset(&pt, 0, sizeof(page_table));
    memset(&kpt, 0, sizeof(page_table));

    // Identity map the first 4MB
    for (uint32_t i = 0, addr = 0x0, virt = 0x0; i < 1024; i++, addr += sizeof(page_table), virt += sizeof(page_table)) {
        page *p = &kpt.entries[(virt >> 12) & 0x3FF];
        p->present = 1;
        p->rw = 1;
        p->address = addr;
    }

    // Map the kernel
    for (uint32_t i = 0, addr = 0x100000, virt = 0xC0000000; i < 1024; i++, addr += sizeof(page_table), virt += sizeof(page_table)) {
        page *p = &pt.entries[(virt >> 12) & 0x3FF];
        p->present = 1;
        p->rw = 1;
        p->address = addr;
    }

    page_directory *pd = &pdt.entries[0xC0000000 >> 22];
    pd->present = 1;
    pd->rw = 1;
    pd->address = (uint32_t) &pt;

    page_directory *kpd = &pdt.entries[0x0 >> 22];
    kpd->present = 1;
    kpd->rw = 1;
    kpd->address = (uint32_t) &kpt;

    isr_install_handler(14, page_fault_handler);

    current_pd = &pdt;

    mmu_enable_paging(&pdt);*/
}

void mmu_install_page(uint32_t addr) {
    /*page_directory[addr / 0x400000] = (page_directory_entry) {
        .present = 1,
        .rw = 1,
        .user = 1,
        .page_frame = addr / 0x1000
    };*/
}

void mmu_invalidate_page(uint32_t addr) {
    asm volatile("invlpg (%0)" : : "r"(addr) : "memory");
}
