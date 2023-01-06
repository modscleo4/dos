#include "cpuid.h"

#define DEBUG 1

#include <string.h>
#include "../debug.h"

bool get_cpuid_info(cpu_info *cpuinfo) {
    memset(cpuinfo, 0, sizeof(cpu_info));
    if (cpuid_available()) {
        // Get Vendor ID
        uint32_t eax, ebx, ecx, edx;
        asm volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(0));

        // copy to cpu_info->vendor_id
        memcpy(&cpuinfo->vendor_id[0], &ebx, 4);
        memcpy(&cpuinfo->vendor_id[4], &edx, 4);
        memcpy(&cpuinfo->vendor_id[8], &ecx, 4);

        // Get Processor Info
        asm volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(1));

        dbgprint("CPUID: eax: %x, ebx: %x, ecx: %x, edx: %x\n", eax, ebx, ecx, edx);

        // copy to cpu_info->ecx
        cpuinfo->ecx = ecx;

        // copy to cpu_info->edx
        cpuinfo->edx = edx;

        return true;
    }

    return false;
}
