#include "process.h"

process processes[10];

uint32_t create_process(uint32_t entry, uint32_t stack) {
    //
}

void destroy_process(uint32_t pid) {
    if (pid < 10) {
        processes[pid].return_address = 0;
        processes[pid].stack_pointer = 0;
        processes[pid].stack_base = 0;
        processes[pid].entry = 0;
    }
}
