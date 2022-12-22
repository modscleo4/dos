#ifndef KERNEL_CPUID_H
#define KERNEL_CPUID_H

#include <stdbool.h>

typedef struct cpu_info {
    char vendor_id[13];

    // ecx features
        bool sse3: 1;
        bool pclmul: 1;
        bool dtes64: 1;
        bool monitor: 1;
        bool ds_cpl: 1;
        bool vmx: 1;
        bool smx: 1;
        bool est: 1;
        bool tm2: 1;
        bool ssse3: 1;
        bool cnxt_id: 1;
        bool sdbg: 1;
        bool fma: 1;
        bool cmpxchg16b: 1;
        bool xtpr: 1;
        bool pdcm: 1;
        bool pcid: 1;
        bool dca: 1;
        bool sse41: 1;
        bool sse42: 1;
        bool x2apic: 1;
        bool movbe: 1;
        bool popcnt: 1;
        bool tsc_deadline: 1;
        bool aes: 1;
        bool xsave: 1;
        bool osxsave: 1;
        bool avx: 1;
        bool f16c: 1;
        bool rdrand: 1;
        bool hypervisor: 1;

    // edx features
        bool fpu: 1;
        bool vme: 1;
        bool de: 1;
        bool pse: 1;
        bool tsc: 1;
        bool msr: 1;
        bool pae: 1;
        bool mce: 1;
        bool cx8: 1;
        bool apic: 1;
        bool sep: 1;
        bool mtrr: 1;
        bool pge: 1;
        bool mca: 1;
        bool cmov: 1;
        bool pat: 1;
        bool pse36: 1;
        bool psn: 1;
        bool clflush: 1;
        bool dts: 1;
        bool acpi: 1;
        bool mmx: 1;
        bool fxsr: 1;
        bool sse: 1;
        bool sse2: 1;
        bool ss: 1;
        bool htt: 1;
        bool tm: 1;
        bool ia64: 1;
        bool pbe: 1;
} __attribute__((packed)) cpu_info;

extern bool cpuid_available(void);

bool get_cpuid_info(cpu_info *cpuinfo);

#endif // KERNEL_CPUID_H
