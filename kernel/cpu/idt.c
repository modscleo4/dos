#include "idt.h"

#include "../bits.h"
#include <string.h>

idt_entry idt[256];
idt_ptr ip;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags | 0x60;
}

void idt_init(void) {
    ip.limit = sizeof(idt) - 1;
    ip.base = (unsigned int) &idt;
    memset(&idt, 0, sizeof(idt));

    load_idt(&ip);
}
