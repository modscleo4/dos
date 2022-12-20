#ifndef PROCESS_H
#define PROCESS_H

typedef struct process {
    unsigned int return_address;
    unsigned int stack_pointer;
    unsigned int stack_base;
    unsigned int entry;
} process;

unsigned long int create_process(unsigned int entry, unsigned int stack);

void destroy_process(unsigned long int pid);

#endif // PROCESS_H
