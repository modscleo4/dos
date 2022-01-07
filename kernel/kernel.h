#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "cpu/fpu.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/irq.h"
#include "cpu/isr.h"
#include "cpu/pic.h"
#include "cpu/syscall.h"
#include "drivers/floppy.h"
#include "drivers/keyboard.h"
#include "drivers/screen.h"
#include "modules/timer.h"

uintptr_t _esp;

extern void switch_ring3(unsigned int);

void panic(const char *msg);

void dbgprint(const char *msg, ...);

void hexdump(void *ptr, size_t n);

void start_shell(void);

void kernel_main(uintptr_t);

#endif //KERNEL_H
