#include "isr.h"

#define DEBUG 1

#include <stdio.h>
#include "idt.h"
#include "panic.h"
#include "pic.h"
#include "../debug.h"
#include "../modules/process.h"

void (*isr_routines[32])(registers *, uint32_t) = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void isr_install_handler(int isr, void (*handler)(registers *, uint32_t)) {
    isr_routines[isr] = handler;
}

void isr_uninstall_handler(int isr) {
    isr_routines[isr] = 0;
}

void isr_init(void) {
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);
    idt_set_gate(127, (uint32_t)isr127, 0x08, 0x8E);
}

static const char *exception_messages[] = {
    "Division-by-zero Error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown interrupt",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security",
    "Reserved"
};

void isr_fault_handler(registers *r) {
    if (r->int_no < 32 || r->int_no == 127) {
        if (r->int_no < 32 && isr_routines[r->int_no]) {
            isr_routines[r->int_no](r, r->int_no);
            return;
        }

        if (r->cs & 0x3) {
            // user mode
            process *p = process_current();
            if (p) {
                process_destroy(p);
                process_switch(process_kernel(), r);

                // only print err_code if the exception set it
                if (r->int_no == 8 || r->int_no == 10 || r->int_no == 11 || r->int_no == 12 || r->int_no == 13 || r->int_no == 14 || r->int_no == 17 || r->int_no == 21 || r->int_no == 29 || r->int_no == 30) {
                    dbgprint("Process %ld terminated due to %d(%s): %d\n", p->pid, r->int_no, exception_messages[r->int_no], r->err_code);
                } else {
                    dbgprint("Process %ld terminated due to %d(%s)\n", p->pid, r->int_no, exception_messages[r->int_no]);
                }
                return;
            }
        }

        // only print err_code if the exception set it
        if (r->int_no == 8 || r->int_no == 10 || r->int_no == 11 || r->int_no == 12 || r->int_no == 13 || r->int_no == 14 || r->int_no == 17 || r->int_no == 21 || r->int_no == 29 || r->int_no == 30) {
            panic_handler(r, "%d: %s", r->err_code, exception_messages[r->int_no]);
        } else {
            panic_handler(r, "%s", exception_messages[r->int_no]);
        }
    }
}
