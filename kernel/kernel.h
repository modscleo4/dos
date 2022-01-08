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

int boot_drive;

extern void switch_ring3(unsigned int);

void load_regs(void);

void panic_handler(const char *);

#define panic(msg) (load_regs(), panic_handler(msg))

void dbgprint(const char *msg, ...);

void hexdump(void *ptr, size_t n);

void kernel_main();

#endif //KERNEL_H
