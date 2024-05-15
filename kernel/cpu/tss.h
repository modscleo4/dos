#ifndef KERNEL_TSS_H
#define KERNEL_TSS_H

#include <stdint.h>

#pragma pack(push, 1)
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
} tss_entry;
#pragma pack(pop)

extern void tss_flush(void);

void install_tss(int num, uint16_t ss0, uint32_t esp0);

void set_kernel_stack(uint32_t stack);

#endif // KERNEL_TSS_H
