#include "syscall.h"
#include "../drivers/keyboard.h"
#include "../drivers/screen.h"

#include <stdarg.h>
#include <stdio.h>

// This var is to store the current SYSCALL return value
int __syscall_ret;

int run_syscall(registers *r) {
    asm("sti");
    int no = r->eax;

    int arg0 = r->ebx;
    int arg1 = r->ecx;
    int arg2 = r->edx;
    int arg3 = r->esi;
    int arg4 = r->edi;
    int arg5 = r->ebp;

    switch (no) {
        case 0:
            return keyboard_read();

        case 1:
            return screen_write(arg0);

        default:
            printf("Invalid SYSCALL\n");
            return -1;
    }
}

void syscall_init() {
    idt_set_gate(128, (unsigned int)syscall_handler, 0x08, 0x8E);
}
