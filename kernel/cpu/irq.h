#ifndef KERNEL_IRQ_H
#define KERNEL_IRQ_H

#include "system.h"

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

extern void irq0(void);

extern void irq1(void);

extern void irq2(void);

extern void irq3(void);

extern void irq4(void);

extern void irq5(void);

extern void irq6(void);

extern void irq7(void);

extern void irq8(void);

extern void irq9(void);

extern void irq10(void);

extern void irq11(void);

extern void irq12(void);

extern void irq13(void);

extern void irq14(void);

extern void irq15(void);

void irq_remap(void);

void irq_install_handler(int, void (*)(registers*));

void irq_uninstall_handler(int);

void irq_init(void);

void irq_handler(registers *);

#endif //KERNEL_IRQ_H
