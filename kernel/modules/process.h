#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

typedef struct process {
    uint32_t return_address;
    uint32_t stack_pointer;
    uint32_t stack_base;
    uint32_t entry;
} process;

uint32_t create_process(uint32_t entry, uint32_t stack);

void destroy_process(uint32_t pid);

#endif // PROCESS_H
