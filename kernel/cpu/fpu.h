#ifndef KERNEL_FPU_H
#define KERNEL_FPU_H

#include <stdbool.h>
#include <stdint.h>
#include "cpuid.h"

enum CR0 {
    CR0_MP = 1 << 1,
    CR0_EM = 1 << 2,
    CR0_TS = 1 << 3,
    CR0_ET = 1 << 4,
    CR0_NE = 1 << 5,
    CR0_WP = 1 << 16,
    CR0_AM = 1 << 18,
    CR0_NW = 1 << 29,
    CR0_CD = 1 << 30,
    CR0_PG = 1 << 31
};

enum CR4 {
    CR4_VME = 1 << 0,
    CR4_PVI = 1 << 1,
    CR4_TSD = 1 << 2,
    CR4_DE = 1 << 3,
    CR4_PSE = 1 << 4,
    CR4_PAE = 1 << 5,
    CR4_MCE = 1 << 6,
    CR4_PGE = 1 << 7,
    CR4_PCE = 1 << 8,
    CR4_OSFXSR = 1 << 9,
    CR4_OSXMMEXCPT = 1 << 10,
    CR4_UMIP = 1 << 11,
    CR4_VMXE = 1 << 13,
    CR4_SMXE = 1 << 14,
    CR4_FSGSBASE = 1 << 16,
    CR4_PCIDE = 1 << 17,
    CR4_OSXSAVE = 1 << 18,
    CR4_SMEP = 1 << 20,
    CR4_SMAP = 1 << 21
};

extern bool fpu_available(void);

void fpu_load_control_word(const uint16_t control);

void fpu_init(cpu_info *cpuinfo);

void sse_init(cpu_info *cpuinfo);

#endif // KERNEL_FPU_H
