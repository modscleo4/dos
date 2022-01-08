#include "syscall.h"

#include "../cpu/gdt.h"
#include "../drivers/keyboard.h"
#include "../drivers/screen.h"
#include "../kernel.h"

#include <stdarg.h>
#include <stdio.h>

// This var is to store the current SYSCALL return value
int __syscall_ret;

int run_syscall(registers *r) {
    int ret = -1;
    //set_kernel_stack(_esp);
    long int no = r->eax;
    asm("sti");

    long int arg0 = r->ebx;
    long int arg1 = r->ecx;
    long int arg2 = r->edx;
    long int arg3 = r->esi;
    long int arg4 = r->edi;
    long int arg5 = r->ebp;
    //dbgprint("syscall: %x(%x %x %x %x %x %x)\n", no, arg0, arg1, arg2, arg3, arg4, arg5);

    switch (no) {
        case 3:
            ret = read(arg0, arg1);
            break;

        case 4:
            ret = write(arg0, arg1) == arg1 ? 0 : -1;
            break;

        default:
            dbgprint("Invalid SYSCALL\n");
            ret = -1;
    }

    //set_kernel_stack(r->esp);
    return ret;
}

void syscall_init() {
    idt_set_gate(128, (unsigned int)syscall_handler, 0x08, 0x8E);
}
