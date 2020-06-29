#ifndef KERNEL_SYSTEM_H
#define KERNEL_SYSTEM_H

typedef struct registers {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
} registers;

#endif //KERNEL_SYSTEM_H
