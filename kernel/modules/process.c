#include "process.h"

#include <stdlib.h>

process *processes;

void process_init(void) {
    processes = (process *) calloc(10, sizeof(process));
}

uint32_t create_process(uint32_t entry, uint32_t stack) {
    return 0;
}

void destroy_process(uint32_t pid) {
    if (pid < 10) {
        processes[pid].return_address = 0;
        processes[pid].stack_pointer = 0;
        processes[pid].stack_base = 0;
        processes[pid].entry = 0;
    }
}
