#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

extern void load_idt(unsigned long int);

typedef struct IDT_entry {
    unsigned short int base_low;
    unsigned short int selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short int base_high;
} __attribute__ ((packed)) IDT_entry;

typedef struct IDT_ptr {
    unsigned short int limit;
    unsigned int base;
} __attribute__ ((packed)) IDT_ptr;

void idt_set_gate(unsigned char, unsigned long int, unsigned short int, unsigned char);

void idt_init(void);

#endif //KERNEL_IDT_H
