#include "gdt.h"

#include <string.h>
#include "tss.h"

static gdt_entry gdt[6];

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].entry.base_low = (base & 0xFFFF);
    gdt[num].entry.base_middle = (base >> 16) & 0xFF;
    gdt[num].entry.base_high = (base >> 24) & 0xFF;

    gdt[num].entry.limit_low = (limit & 0xFFFF);
    gdt[num].entry.granularity = ((limit >> 16) & 0x0F);

    gdt[num].entry.granularity |= (gran & 0xF0);
    gdt[num].entry.access = access;
}

void gdt_init(void) {
    gdt_ptr gp;
    gp.base = (uint32_t)gdt;
    gp.limit = sizeof(gdt) - 1;

    // NS
    gdt_set_gate(0, 0, 0, 0, 0);
    // CS
    gdt_set_gate(1, 0, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_SEGMENT | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_READ_WRITE, 0xCF);
    // DS
    gdt_set_gate(2, 0, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_SEGMENT | GDT_ACCESS_DATA | GDT_ACCESS_READ_WRITE, 0xCF);
    // Ring3 CS
    gdt_set_gate(3, 0, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_SEGMENT | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_READ_WRITE, 0xCF);
    // Ring3 DS
    gdt_set_gate(4, 0, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_SEGMENT | GDT_ACCESS_DATA | GDT_ACCESS_READ_WRITE, 0xCF);

    install_tss(5, 0x10, 0x0);

    //gdt_set_gate(6, 0, 0xFFFFFFFF, 0x92, 0x0);
    //gdt_set_gate(7, 0, 0xFFFFFFFF, 0x9A, 0x0);

    gdt_flush(&gp);
    tss_flush();
}
