#include "process.h"

process processes[10];

unsigned long int create_process(unsigned int entry, unsigned int stack) {
    //
}

void destroy_process(unsigned long int pid) {
    if (pid < 10) {
        processes[pid].return_address = 0;
        processes[pid].stack_pointer = 0;
        processes[pid].stack_base = 0;
        processes[pid].entry = 0;
    }
}
