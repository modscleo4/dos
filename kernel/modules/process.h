#ifndef PROCESS_H
#define PROCESS_H

typedef struct process {
    unsigned int return_address;
    unsigned int stack_pointer;
    unsigned int stack_base;
    unsigned int entry;
} process;

#endif // PROCESS_H
