#include "fpu.h"

#define DEBUG 1

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "../bits.h"
#include "../debug.h"

void fpu_load_control_word(const uint16_t control) {
    asm volatile("fldcw %0;"
                 :
                 : "m"(control));
}

void fpu_init(void) {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0;"
                 : "=r"(cr0));
    if (fpu_available()) {
        dbgprint("FPU available\n");
        cr0 = DISABLE_BIT_INT(cr0, CR0_EM);
        cr0 = DISABLE_BIT_INT(cr0, CR0_TS);

        asm volatile("mov %0, %%cr0;"
                     :
                     : "r"(cr0));

        //fpu_load_control_word(0x37F);

        sse_init();
    } else {
        dbgprint("FPU not available\n");
        cr0 = ENABLE_BIT_INT(cr0, CR0_EM);

        asm volatile("mov %0, %%cr0;"
                     :
                     : "r"(cr0));
    }
}

void sse_init(void) {
    uint32_t cr0;
    uint32_t cr4;
    asm volatile("mov %%cr0, %0;"
                 : "=r"(cr0));
    asm volatile("mov %%cr4, %0;"
                 : "=r"(cr4));

    if (ISSET_BIT_INT(cpuinfo.edx, CPUID_FEAT_EDX_SSE)) {
        dbgprint("SSE available\n");
        cr0 = DISABLE_BIT_INT(cr0, CR0_EM);
        cr0 = ENABLE_BIT_INT(cr0, CR0_MP);
        cr4 = ENABLE_BIT_INT(cr4, CR4_OSFXSR);
        cr4 = ENABLE_BIT_INT(cr4, CR4_OSXMMEXCPT);
        asm volatile("mov %0, %%cr0;"
                     :
                     : "r"(cr0));
        asm volatile("mov %0, %%cr4;"
                     :
                     : "r"(cr4));

        double a = 1.0;
        double b = 2.0;
        dbgprint("Testing SSE: %lf\n", a - b);
    } else {
        dbgprint("SSE not available\n");
    }
}
