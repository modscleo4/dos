#ifndef KERNEL_IRQ_H
#define KERNEL_IRQ_H

#include "system.h"
#include "idt.h"
#include "pic.h"
#include "../bits.h"

enum IRQ {
    IRQ_PIT = 0,
    IRQ_KEYBOARD,
    IRQ_CASCADE,
    IRQ_COM2,
    IRQ_COM1,
    IRQ_LPT2,
    IRQ_FLOPPY,
    IRQ_LPT1,
    IRQ_CMOS,
    IRQ_FREE_0,
    IRQ_FREE_1,
    IRQ_FREE_2,
    IRQ_PS2_MOUSE,
    IRQ_FPU,
    IRQ_ATA_PRIMARY,
    IRQ_ATA_SECONDARY
};

extern void irq0();

extern void irq1();

extern void irq2();

extern void irq3();

extern void irq4();

extern void irq5();

extern void irq6();

extern void irq7();

extern void irq8();

extern void irq9();

extern void irq10();

extern void irq11();

extern void irq12();

extern void irq13();

extern void irq14();

extern void irq15();

void irq_remap();

void irq_install_handler(int, void (*)(registers*));

void irq_uninstall_handler(int);

void irq_init();

void irq_handler(registers*);

#endif //KERNEL_IRQ_H
