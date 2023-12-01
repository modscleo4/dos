#include "syscall.h"

#define DEBUG 1

#include <stdarg.h>
#include <stdio.h>
#include "gdt.h"
#include "idt.h"
#include "interrupts.h"
#include "panic.h"
#include "system.h"
#include "../debug.h"
#include "../ring3.h"
#include "../drivers/ethernet.h"
#include "../modules/net/dns.h"
#include "../modules/net/tcp.h"
#include "../modules/net/udp.h"

// This var is to store the current SYSCALL return value
long int __syscall_ret;

static long int syscall_exit(registers *r, int exit_code) {
    panic_handler(r, "Process exited with code %d\n", exit_code);
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

static long int syscall_read(registers *r, int fd, void *buf, size_t count) {
    return read(fd, buf, count);
}

static long int syscall_write(registers *r, int fd, void *buf, size_t count) {
    return write(fd, buf, count);
}

static long int syscall_dns(registers *r, int eth_driver, char* type, char *domain, uint8_t *ip) {
    return dns_query_ipv4(eth[eth_driver], eth[eth_driver]->ipv4.dns, domain, ip, 1000);
}

static long int syscall_listen_tcp(registers *r, uint16_t port, tcp_listener callback) {
    return tcp_install_listener(port, callback);
}

static long int syscall_listen_udp(registers *r, uint16_t port, udp_listener callback) {
    return udp_install_listener(port, callback);
}

static void *syscalls[] = {
    &syscall_exit,
    NULL,
    NULL,
    &syscall_read,
    &syscall_write,
    &syscall_dns,
    &syscall_listen_tcp,
    &syscall_listen_udp,
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
    uint32_t esp;
    int ret = -1;
    interrupts_reenable();
    uint32_t no = r->eax;

    uint32_t arg0 = r->ebx;
    uint32_t arg1 = r->ecx;
    uint32_t arg2 = r->edx;
    uint32_t arg3 = r->esi;
    uint32_t arg4 = r->edi;
    uint32_t arg5 = r->ebp;
    //dbgprint("syscall: %x(%x, %x, %x, %x, %x, %x)\n", no, arg0, arg1, arg2, arg3, arg4, arg5);

    if (syscalls[no]) {
        uint32_t (*syscall_fn)(registers *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) = syscalls[no];
        ret = syscall_fn(r, arg0, arg1, arg2, arg3, arg4, arg5);
    } else {
        dbgprint("syscall: %x not implemented\n", no);
        ret = -1;
    }

    return ret;
}

void syscall_init(void) {
    idt_set_gate(128, (uint32_t)syscall_handler, 0x08, 0x8E);
}
