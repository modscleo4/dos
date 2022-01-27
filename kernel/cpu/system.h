#ifndef KERNEL_SYSTEM_H
#define KERNEL_SYSTEM_H

#include <stdbool.h>

typedef struct eflags {
    bool carry: 1;
    unsigned char reserved0: 1;
    bool parity: 1;
    unsigned char reserved1: 1;
    bool adjust: 1;
    unsigned char reserved2: 1;
    bool zero: 1;
    bool sign: 1;
    bool trap: 1;
    bool interrupt: 1;
    bool direction: 1;
    bool overflow: 1;
    unsigned char iopl : 2;
    bool nt: 1;
    unsigned char reserved3: 1;
    bool resume: 1;
    bool virtual_86: 1;
    bool alignment: 1;
    bool virtual_interrupt: 1;
    bool virtual_interrupt_pending: 1;
    bool id: 1;
    unsigned int reserved4: 10;
} __attribute__((packed)) eflags;

typedef struct registers {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs;
    eflags eflags;
    unsigned int useresp, ss;
} registers;

#endif //KERNEL_SYSTEM_H
