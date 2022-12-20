#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H

#include <stdint.h>

typedef union gdt_entry {
    struct {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_middle;
        uint8_t access;
        uint8_t granularity;
        uint8_t base_high;
    } __attribute__((packed)) entry;
    struct {
        uint32_t limit_low : 16;
        uint32_t base_low : 24;
        uint8_t accessed : 1;
        uint8_t read_write : 1;             // readable for code, writable for data
        uint8_t conforming_expand_down : 1; // conforming for code, expand down for data
        uint8_t code : 1;                  // 1 for code, 0 for data
        uint8_t code_data_segment : 1;     // should be 1 for everything but TSS and LDT
        uint8_t DPL : 2;                    // privilege level
        uint8_t present : 1;
        uint8_t limit_high : 4;
        uint8_t available : 1; // only used in software; has no effect on hardware
        uint8_t long_mode : 1;
        uint8_t big : 1; // 32-bit opcodes for code, uint32_t stack for data
        uint8_t granularity : 1; // 1 to use 4k page addressing, 0 for byte addressing
        uint8_t base_high : 8;
    } __attribute__((packed)) bits;
} __attribute__((packed)) gdt_entry;

typedef struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr;

typedef struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) tss_entry;

extern void gdt_flush(gdt_ptr *ptr);

extern void tss_flush(void);

void gdt_init(void);

void install_tss(int num, uint16_t ss0, uint32_t esp0);

void set_kernel_stack(uint32_t stack);

#endif //KERNEL_GDT_H
