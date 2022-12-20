#include "gdt.h"
#include <string.h>

gdt_entry entry[6];
gdt_ptr gp;
tss_entry tss;

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    entry[num].entry.base_low = (base & 0xFFFF);
    entry[num].entry.base_middle = (base >> 16) & 0xFF;
    entry[num].entry.base_high = (base >> 24) & 0xFF;

    entry[num].entry.limit_low = (limit & 0xFFFF);
    entry[num].entry.granularity = ((limit >> 16) & 0x0F);

    entry[num].entry.granularity |= (gran & 0xF0);
    entry[num].entry.access = access;
}

void gdt_init(void) {
    gp.limit = sizeof(entry) - 1;
    gp.base = (unsigned int)&entry;

    // NS
    gdt_set_gate(0, 0, 0, 0, 0);
    // CS
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    // DS
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    // Ring3 CS
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    // Ring3 DS
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    install_tss(5, 0x10, 0x0);

    //gdt_set_gate(6, 0, 0xFFFFFFFF, 0x92, 0x0);
    //gdt_set_gate(7, 0, 0xFFFFFFFF, 0x9A, 0x0);

    gdt_flush(&gp);
    tss_flush();
}

void install_tss(int num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = base + sizeof(tss);

    gdt_set_gate(num, base, limit, 0xE9, 0x00);

    memset(&tss, 0x0, sizeof(tss));

    tss.ss0 = ss0;
    tss.esp0 = esp0;
    tss.cs = 0x0b;
    tss.ss = 0x13;
    tss.ds = 0x13;
    tss.es = 0x13;
    tss.fs = 0x13;
    tss.gs = 0x13;

    tss.iomap_base = sizeof(tss);
}

void set_kernel_stack(unsigned int stack) {
    tss.esp0 = stack;
}
