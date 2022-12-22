#ifndef KERNEL_SYSTEM_H
#define KERNEL_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>

typedef struct eflags {
    bool carry: 1;
    uint8_t reserved0: 1;
    bool parity: 1;
    uint8_t reserved1: 1;
    bool adjust: 1;
    uint8_t reserved2: 1;
    bool zero: 1;
    bool sign: 1;
    bool trap: 1;
    bool interrupt: 1;
    bool direction: 1;
    bool overflow: 1;
    uint8_t iopl : 2;
    bool nt: 1;
    uint8_t reserved3: 1;
    bool resume: 1;
    bool virtual_86: 1;
    bool alignment: 1;
    bool virtual_interrupt: 1;
    bool virtual_interrupt_pending: 1;
    bool id: 1;
    uint16_t reserved4: 10;
} __attribute__((packed)) eflags;

/**
 * All registers should be 32 bit, since asm "push" moves the stack 32 bit for any value
 */
typedef struct registers {
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t int_no;
    uint32_t err_code;

    uint32_t eip;
    uint32_t cs;

    eflags eflags;

    uint32_t useresp;
    uint32_t ss;
} registers;

#endif //KERNEL_SYSTEM_H
