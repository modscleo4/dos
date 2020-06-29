#include "gdt.h"

GDT_entry entry[3];
GDT_ptr gp;

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
    gp.base = (unsigned int) &entry;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_flush((unsigned long int) &gp);
}
