#include "syscall.h"

#include "../cpu/gdt.h"
#include "../debug.h"
#include "../ring3.h"
#include "../drivers/keyboard.h"
#include "../drivers/screen.h"
#include "idt.h"
#include "system.h"

#include <stdarg.h>
#include <stdio.h>

// This var is to store the current SYSCALL return value
int __syscall_ret;

long int syscall_exit(int exit_code) {
    return 0;
}

long int syscall_read(int fd, char *buf, size_t count) {
    return read(buf, count);
}

long int syscall_write(int fd, char *buf, size_t count) {
    return write(buf, count);
}

static void *syscalls[] = {
    &syscall_exit,
    NULL,
    NULL,
    &syscall_read,
    &syscall_write,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

int run_syscall(registers *r) {
    unsigned long int esp;
    int ret = -1;
    asm("sti");
    long int no = r->eax;

    long int arg0 = r->ebx;
    long int arg1 = r->ecx;
    long int arg2 = r->edx;
    long int arg3 = r->esi;
    long int arg4 = r->edi;
    long int arg5 = r->ebp;
    //dbgprint("syscall: %x(%x %x %x %x %x %x)\n", no, arg0, arg1, arg2, arg3, arg4, arg5);

    if (syscalls[no]) {
        long int (*syscall_fn)(long int, long int, long int, long int, long int, long int) = syscalls[no];
        ret = syscall_fn(arg0, arg1, arg2, arg3, arg4, arg5);
        asm("add $4, %esp");
        asm("add $4, %esp");
        asm("add $4, %esp");
        asm("add $4, %esp");
        asm("add $4, %esp");
        asm("add $4, %esp");
    } else {
        dbgprint("syscall: %x not implemented\n", no);
        ret = -1;
    }

    return ret;
}

void syscall_init(void) {
    idt_set_gate(128, (unsigned int)syscall_handler, 0x08, 0x8E);
}
