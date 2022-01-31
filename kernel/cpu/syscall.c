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

static long int syscall_exit(registers *r, int exit_code) {
    // Stay on ring0
    asm volatile("mov %%cs, %0" : "=r"(r->cs));
    asm volatile("mov %%ds, %0" : "=r"(r->ds));
    asm volatile("mov %%es, %0" : "=r"(r->es));
    asm volatile("mov %%fs, %0" : "=r"(r->fs));
    asm volatile("mov %%gs, %0" : "=r"(r->gs));
    asm volatile("mov %%ss, %0" : "=r"(r->ss));

    r->eip = r->edi;

    asm volatile("pushf; pop %0" : "=r"(r->eflags));

    return exit_code;
}

static long int syscall_read(registers *r, int fd, char *buf, size_t count) {
    return read(buf, count);
}

static long int syscall_write(registers *r, int fd, char *buf, size_t count) {
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
        long int (*syscall_fn)(registers *, long int, long int, long int, long int, long int, long int) = syscalls[no];
        ret = syscall_fn(r, arg0, arg1, arg2, arg3, arg4, arg5);
    } else {
        dbgprint("syscall: %x not implemented\n", no);
        ret = -1;
    }

    return ret;
}

void syscall_init(void) {
    idt_set_gate(128, (unsigned int)syscall_handler, 0x08, 0x8E);
}
