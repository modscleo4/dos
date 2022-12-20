#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include <stdint.h>

typedef struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__ ((packed)) idt_entry;

typedef struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr;

extern void load_idt(idt_ptr *ptr);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

void idt_init(void);

#endif //KERNEL_IDT_H
