#include "fpu.h"

#include "cpuid.h"
#include "../bits.h"

void fpu_load_control_word(const unsigned short int control) {
    asm volatile("fldcw %0;"
                 :
                 : "m"(control));
}

void fpu_init() {
    long int cr0;
    asm volatile("mov %%cr0, %0;"
                 : "=r"(cr0));
    DISABLE_BIT_INT(cr0, CR0_EM);
    DISABLE_BIT_INT(cr0, CR0_TS);
    if (cpuid.fpu || fpu_available()) {
        dbgprint("FPU available\n");
        //fpu_load_control_word(0x37F);
        //fpu_load_control_word(0x37E);
        //fpu_load_control_word(0x37A);

        sse_init();
    } else {
        dbgprint("FPU not available\n");
        ENABLE_BIT_INT(cr0, CR0_EM);
    }
}

bool sse_available() {
    return cpuid.sse;
}

void sse_init() {
    long int cr0;
    long int cr4;
    asm volatile("mov %%cr0, %0;"
                 : "=r"(cr0));
    asm volatile("mov %%cr4, %0;"
                 : "=r"(cr4));
    if (sse_available()) {
        dbgprint("SSE available\n");
        ENABLE_BIT_INT(cr0, CR0_MP);
        ENABLE_BIT_INT(cr4, CR4_OSFXSR);
        ENABLE_BIT_INT(cr4, CR4_OSXMMEXCPT);
        asm volatile("mov %0, %%cr0;"
                     :
                     : "r"(cr0));
        asm volatile("mov %0, %%cr4;"
                     :
                     : "r"(cr4));
        double a = 1.0;
        double b = 2.0;
        printf("Testing SSE: %lf\n", a - b);
    } else {
        dbgprint("SSE not available\n");
    }
}
