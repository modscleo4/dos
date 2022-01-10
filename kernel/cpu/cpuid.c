#include "cpuid.h"

#include <string.h>

bool get_cpuid_info(cpu_info *i) {
    memset(i, 0, sizeof(cpu_info));
    if (cpuid_available()) {
        // Get Vendor ID
        unsigned int eax, ebx, ecx, edx;
        asm volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(0));

        char buf[sizeof(cpu_info)];

        // copy to cpu_info->vendor_id
        memcpy(buf, &ebx, 4);
        memcpy(buf + 4, &edx, 4);
        memcpy(buf + 8, &ecx, 4);
        buf[12] = 0;

        // Get Processor Info
        asm volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(1));

        // copy to cpu_info->ecx
        memcpy(buf + 13, &ecx, 4);

        // copy to cpu_info->edx
        memcpy(buf + 13 + 4, &edx, 4);

        memcpy(i, buf, sizeof(cpu_info));

        return true;
    }

    return false;
}
