#ifndef KERNEL_FPU_H
#define KERNEL_FPU_H

#include <stdbool.h>
#include <stdint.h>
#include "cpuid.h"

enum CR0 {
    CR0_MP = 1UL << 1UL,
    CR0_EM = 1UL << 2UL,
    CR0_TS = 1UL << 3UL,
    CR0_ET = 1UL << 4UL,
    CR0_NE = 1UL << 5UL,
    CR0_WP = 1UL << 16UL,
    CR0_AM = 1UL << 18UL,
    CR0_NW = 1UL << 29UL,
    CR0_CD = 1UL << 30UL,
    CR0_PG = 1UL << 31UL
};

enum CR4 {
    CR4_VME = 1UL << 0UL,
    CR4_PVI = 1UL << 1UL,
    CR4_TSD = 1UL << 2UL,
    CR4_DE = 1UL << 3UL,
    CR4_PSE = 1UL << 4UL,
    CR4_PAE = 1UL << 5UL,
    CR4_MCE = 1UL << 6UL,
    CR4_PGE = 1UL << 7UL,
    CR4_PCE = 1UL << 8UL,
    CR4_OSFXSR = 1UL << 9UL,
    CR4_OSXMMEXCPT = 1UL << 10UL,
    CR4_UMIP = 1UL << 11UL,
    CR4_VMXE = 1UL << 13UL,
    CR4_SMXE = 1UL << 14UL,
    CR4_FSGSBASE = 1UL << 16UL,
    CR4_PCIDE = 1UL << 17UL,
    CR4_OSXSAVE = 1UL << 18UL,
    CR4_SMEP = 1UL << 20UL,
    CR4_SMAP = 1UL << 21UL
};

extern bool fpu_available(void);

void fpu_load_control_word(const uint16_t control);

void fpu_init(cpu_info *cpuinfo);

void sse_init(cpu_info *cpuinfo);

#endif // KERNEL_FPU_H
