#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include <stdlib.h>
#include "drivers/screen.h"
#include "drivers/floppy.h"
#include "drivers/keyboard.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/isr.h"
#include "cpu/irq.h"
#include "cpu/pic.h"
#include "modules/timer.h"

extern void *sys_stack;

void kernel_main(void);

#endif //KERNEL_H
