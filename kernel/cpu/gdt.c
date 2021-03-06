#include "gdt.h"
#include <string.h>

GDT_entry entry[6];
GDT_ptr gp;
TSS_entry tss;

void gdt_set_gate(int num, unsigned long int base, unsigned long int limit, unsigned char access, unsigned char gran) {
    entry[num].base_low = (base & 0xFFFF);
    entry[num].base_middle = (base >> 16) & 0xFF;
    entry[num].base_high = (base >> 24) & 0xFF;

    entry[num].limit_low = (limit & 0xFFFF);
    entry[num].granularity = ((limit >> 16) & 0x0F);

    entry[num].granularity |= (gran & 0xF0);
    entry[num].access = access;
}

void gdt_init() {
    gp.limit = sizeof(entry) - 1;
    gp.base = (unsigned int)&entry;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    install_tss(5, 0x10, 0x0);

    gdt_flush((unsigned long int)&gp);
    tss_flush();
}

void install_tss(int num, unsigned short int ss0, unsigned int esp0) {
    unsigned int base = (unsigned int)&tss;
    unsigned int limit = base + sizeof(tss);

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
