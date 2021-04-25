#include "irq.h"
#include "gdt.h"

void irq_remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void *irq_routines[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void irq_install_handler(int irq, void (*handler)(registers *)) {
    irq_routines[irq] = handler;
}

void irq_uninstall_handler(int irq) {
    irq_routines[irq] = 0;
}

void irq_init() {
    irq_remap();

    idt_set_gate(32, (unsigned int)irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned int)irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned int)irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned int)irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned int)irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned int)irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned int)irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned int)irq7, 0x08, 0x8E);
    idt_set_gate(40, (unsigned int)irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned int)irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned int)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned int)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned int)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned int)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned int)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned int)irq15, 0x08, 0x8E);
}

void irq_handler(registers *r) {
    void (*handler)(registers *);
    //set_kernel_stack(&r->esp);

    handler = irq_routines[r->int_no - 32];
    if (handler) {
        handler(r);
    }

    PIC_sendEOI(r->int_no);
}
