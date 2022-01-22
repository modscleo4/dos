#ifndef KERNEL_H
#define KERNEL_H

#include "ring3.h"
#include "bits.h"
#include "cpu/cpuid.h"
#include "cpu/fpu.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/irq.h"
#include "cpu/isr.h"
#include "cpu/panic.h"
#include "cpu/pic.h"
#include "cpu/syscall.h"
#include "debug.h"
#include "drivers/pci.h"
#include "drivers/ata.h"
#include "drivers/fat.h"
#include "drivers/floppy.h"
#include "drivers/mbr.h"
#include "drivers/keyboard.h"
#include "drivers/screen.h"
#include "modules/timer.h"
#include "modules/multiboot2.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void kernel_main(unsigned long int, unsigned long int);

#endif //KERNEL_H
