#include "isr.h"
#include <stdio.h>

void isr_init() {
    idt_set_gate(0, (unsigned int) isr0, 0x08, 0x8E);
    idt_set_gate(1, (unsigned int) isr1, 0x08, 0x8E);
    idt_set_gate(2, (unsigned int) isr2, 0x08, 0x8E);
    idt_set_gate(3, (unsigned int) isr3, 0x08, 0x8E);
    idt_set_gate(4, (unsigned int) isr4, 0x08, 0x8E);
    idt_set_gate(5, (unsigned int) isr5, 0x08, 0x8E);
    idt_set_gate(6, (unsigned int) isr6, 0x08, 0x8E);
    idt_set_gate(7, (unsigned int) isr7, 0x08, 0x8E);
    idt_set_gate(8, (unsigned int) isr8, 0x08, 0x8E);
    idt_set_gate(9, (unsigned int) isr9, 0x08, 0x8E);
    idt_set_gate(10, (unsigned int) isr10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned int) isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned int) isr12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned int) isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned int) isr14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned int) isr15, 0x08, 0x8E);
    idt_set_gate(16, (unsigned int) isr16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned int) isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned int) isr18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned int) isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned int) isr20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned int) isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned int) isr22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned int) isr23, 0x08, 0x8E);
    idt_set_gate(24, (unsigned int) isr24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned int) isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned int) isr26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned int) isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned int) isr28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned int) isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned int) isr30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned int) isr31, 0x08, 0x8E);
    idt_set_gate(127, (unsigned int) isr127, 0x08, 0x8E);
}

char *exception_messages[] = {
    "Division by zero",
    "Debug",
    "Non-maskable interrupt",
    "Breakpoint",
    "Detected overflow",
    "Out-of-bounds",
    "Invalid opcode",
    "No coprocessor",
    "Double fault",
    "Coprocessor segment overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Unknown interrupt",
    "Coprocessor fault",
    "Alignment check",
    "Machine check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_fault_handler(registers *r) {
    if (r->int_no < 32 || r->int_no == 127) {
        printf("%s Exception. System Halted!\n", exception_messages[r->int_no]);
        asm("hlt");
    }

    PIC_sendEOI(r->int_no);
}
