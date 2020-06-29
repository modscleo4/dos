#include "idt.h"
#include <string.h>

IDT_entry idt[256];
IDT_ptr ip;

void idt_set_gate(unsigned char num, unsigned long int base, unsigned short int sel, unsigned char flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

void idt_init(void) {
    ip.limit = sizeof(idt) - 1;
    ip.base = (unsigned int) &idt;
    memset(&idt, 0, sizeof(idt));

    load_idt((unsigned long int) &ip);
}
