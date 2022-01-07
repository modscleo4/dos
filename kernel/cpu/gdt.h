#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H

extern void gdt_flush(unsigned long int);

extern void tss_flush();

typedef union GDT_entry {
    struct {
        unsigned short int limit_low;
        unsigned short int base_low;
        unsigned char base_middle;
        unsigned char access;
        unsigned char granularity;
        unsigned char base_high;
    } __attribute__((packed)) entry;
    struct {
        unsigned int limit_low : 16;
        unsigned int base_low : 24;
        unsigned int accessed : 1;
        unsigned int read_write : 1;             // readable for code, writable for data
        unsigned int conforming_expand_down : 1; // conforming for code, expand down for data
        unsigned int code : 1;                   // 1 for code, 0 for data
        unsigned int code_data_segment : 1;      // should be 1 for everything but TSS and LDT
        unsigned int DPL : 2;                    // privilege level
        unsigned int present : 1;
        unsigned int limit_high : 4;
        unsigned int available : 1; // only used in software; has no effect on hardware
        unsigned int long_mode : 1;
        unsigned int big : 1;  // 32-bit opcodes for code, uint32_t stack for data
        unsigned int gran : 1; // 1 to use 4k page addressing, 0 for byte addressing
        unsigned int base_high : 8;
    } __attribute__((packed)) bits;
} __attribute__((packed)) GDT_entry;

typedef struct GDT_ptr {
    unsigned short int limit;
    unsigned int base;
} __attribute__((packed)) GDT_ptr;

typedef struct TSS_entry {
    unsigned int prev_tss;
    unsigned int esp0;
    unsigned int ss0;
    unsigned int esp1;
    unsigned int ss1;
    unsigned int esp2;
    unsigned int ss2;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    unsigned int fs;
    unsigned int gs;
    unsigned int ldt;
    unsigned short int trap;
    unsigned short int iomap_base;
} __attribute__((packed)) TSS_entry;

void gdt_init();

void install_tss(int, unsigned short int, unsigned int);

void set_kernel_stack(unsigned int stack);

#endif //KERNEL_GDT_H
