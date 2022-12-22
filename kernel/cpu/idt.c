#include "idt.h"

#include "../bits.h"
#include <string.h>

idt_entry idt[256];

void idt_set_gate(int num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags | 0x60;
}

void idt_init(void) {
    idt_ptr ip;
    ip.base = (uint32_t)idt;
    ip.limit = sizeof(idt) - 1;

    memset(idt, 0, sizeof(idt));

    load_idt(&ip);
}
