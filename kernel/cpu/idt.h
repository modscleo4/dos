#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include <stdint.h>

#pragma pack(push, 1)
typedef struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} idt_entry;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} idt_ptr;
#pragma pack(pop)

extern void load_idt(idt_ptr *ptr);

void idt_set_gate(int num, uint32_t base, uint16_t sel, uint8_t flags);

void idt_init(void);

#endif //KERNEL_IDT_H
