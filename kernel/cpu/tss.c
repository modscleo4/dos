#include "tss.h"

#include <string.h>
#include "gdt.h"

tss_entry tss;

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

void set_kernel_stack(uint32_t stack) {
    tss.esp0 = stack;
}
