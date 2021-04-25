#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include "idt.h"
#include "system.h"

extern void syscall_handler();

void syscall_init();

#endif //KERNEL_SYSCALL_H
