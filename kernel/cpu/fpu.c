#include "fpu.h"

void fpu_load_control_word(const unsigned short int control) {
    asm volatile("fldcw %0;"
                 :
                 : "m"(control));
}

void fpu_init() {
    size_t cr4;
    asm volatile("mov %%cr4, %0"
                 : "=r"(cr4));
    cr4 |= 0x200;
    asm volatile("mov %0, %%cr4" ::"r"(cr4));
    fpu_load_control_word(0x37F);
}
