#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H

extern void gdt_flush(unsigned long int);

typedef struct GDT_entry {
    unsigned short int limit_low;
    unsigned short int base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed)) GDT_entry;

typedef struct GDT_ptr {
    unsigned short int limit;
    unsigned int base;
} __attribute__((packed)) GDT_ptr;

void gdt_init();

#endif //KERNEL_GDT_H
