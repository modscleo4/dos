#include "syscall.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "gdt.h"
#include "idt.h"
#include "interrupts.h"
#include "panic.h"
#include "system.h"
#include "../debug.h"
#include "../ring3.h"
#include "../drivers/ethernet.h"
#include "../modules/process.h"
#include "../modules/net/dns.h"
#include "../modules/net/tcp.h"
#include "../modules/net/udp.h"

// This var is to store the current SYSCALL return value
long int __syscall_ret;

static inline void syscall_sleep_proc(registers *r, process *p) {
    process_switch(process_kernel(), r);
    p->suspended = true;
    process_disable_round_robin();
    interrupts_reenable();
}

static inline void syscall_wake_proc(registers *r, process *p) {
    interrupts_disable();
    process_enable_round_robin();
    process_switch(p, r);
    p->suspended = false;
}

static long int syscall_read(registers *r, int fd, void *buf, size_t count) {
    process *p = process_current();

    // Switch to ring0, because read() may have to wait for data
    syscall_sleep_proc(r, p);

    int ret = read(
        &p->fd[fd],
        (void *)mmu_get_physical_address_pdt((uintptr_t)buf, p->pdt),
        count
    );

    if (ret < 0) {
        dbgprint("read failed: %d\n", ret);
    } else {
        dbgprint("read %d bytes\n", ret);
    }

    syscall_wake_proc(r, p);

    return ret;
}

static long int syscall_write(registers *r, int fd, void *buf, size_t count) {
    process *p = process_current();

    // Switch to ring0, because write() may have to wait for data
    syscall_sleep_proc(r, p);

    int ret = write(
        &p->fd[fd],
        (void *)mmu_get_physical_address_pdt((uintptr_t)buf, p->pdt),
        count
    );

    if (ret < 0) {
        dbgprint("write failed: %d\n", ret);
    }

    syscall_wake_proc(r, p);

    return ret;
}

static long int syscall_open(registers *r, char *path, int flags) {
    process *p = process_current();

    // Switch to ring0, because open() may have to wait for data
    syscall_sleep_proc(r, p);

    int fd = process_open_file(
        p,
        (char *)mmu_get_physical_address_pdt((uintptr_t)path, p->pdt),
        flags
    );

    syscall_wake_proc(r, p);
    return fd;
}

static long int syscall_close(registers *r, int fd) {
    process *p = process_current();
    return process_close_file(p, fd);
}

static long int syscall_stat(registers *r, const char *filename, struct stat *statbuf) {
    process *p = process_current();

    // Switch to ring0, because stat() may have to wait for data
    syscall_sleep_proc(r, p);

    int ret = process_stat_file(
        p,
        (const char *)mmu_get_physical_address_pdt((uintptr_t)filename, p->pdt),
        statbuf
    );

    syscall_wake_proc(r, p);
    return ret;
}

static long int syscall_fstat(registers *r, unsigned int fd, struct stat *statbuf) {
    process *p = process_current();

    if (fd < 0 || fd >= p->fd_count || !p->fd[fd].used) {
        return -EBADFD;
    }

    file_descriptor *f = &p->fd[fd];
    memcpy((struct stat *)mmu_get_physical_address_pdt((uintptr_t)statbuf, p->pdt), &f->st, sizeof(struct stat) - sizeof(f->st.st_private));

    return 0;
}

static long int syscall_lstat(registers *r, const char *filename, struct stat *statbuf) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_poll(registers *r, /*struct pollfd*/ void *fds, unsigned long nfds, int timeout) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_lseek(registers *r, unsigned int fd, /*off_t*/ long int offset, int whence) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_mmap(registers *r, void *addr, size_t length, int prot, int flags, int fd, long offset) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_mprotect(registers *r, void *addr, size_t length, int prot) {
    process *p = process_current();
    return -1; // Not implemented
}


static long int syscall_munmap(registers *r, void *addr, size_t length) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_brk(registers *r, void *addr) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_pause(registers *r) {
    process *p = process_current();
    process_switch(process_kernel(), r);
    return -1; // Not implemented
}

static long int syscall_getpid(registers *r) {
    process *p = process_current();
    return p->pid;
}

static long int syscall_socket(registers *r, int domain, int type, int protocol) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_connect(registers *r, int sockfd, const /*struct sockaddr*/ void *addr, /*socklen_t*/ size_t addrlen) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_accept(registers *r, int sockfd, /*struct sockaddr*/ void * restrict addr, /*socklen_t*/ void * restrict addrlen) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_shutdown(registers *r, int sockfd, int how) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_bind(registers *r, int sockfd, const /*struct sockaddr*/ void *addr, /*socklen_t*/ size_t addrlen) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_listen(registers *r, int sockfd, int backlog) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_fork(registers *r) {
    process *p = process_current();
    process *child = process_fork(p);
    if (!child) {
        return -1;
    }

    // Child process returns 0
    child->r.eax = 0;

    return child->pid;
}

static long int syscall_execve(registers *r, const char *filename, char *const argv[], char *const envp[]) {
    process *p = process_current();

    syscall_sleep_proc(r, p);

    int ret = process_execve(
        p,
        r,
        (const char *)mmu_get_physical_address_pdt((uintptr_t)filename, p->pdt),
        (char *const *)mmu_get_physical_address_pdt((uintptr_t)argv, p->pdt),
        (char *const *)mmu_get_physical_address_pdt((uintptr_t)envp, p->pdt)
    );

    syscall_wake_proc(r, p);

    return ret;
}

static long int syscall_exit(registers *r, int exit_code) {
    process *p = process_current();
    process_destroy(p);

    process_switch(process_kernel(), r);

    return exit_code;
}

static long int syscall_getcwd(registers *r, char *buf, size_t size) {
    process *p = process_current();
    strncpy(buf, (char *)mmu_get_physical_address_pdt((uintptr_t)buf, p->pdt), size);
    return (long int)buf;
}

static long int syscall_chdir(registers *r, const char *path) {
    process *p = process_current();

    syscall_sleep_proc(r, p);

    int ret = 0;
    strcpy(p->cwd, (char *)mmu_get_physical_address_pdt((uintptr_t)path, p->pdt));

    syscall_wake_proc(r, p);

    return ret;
}

static long int syscall_fchdir(registers *r, int fd) {
    process *p = process_current();

    if (fd < 0 || fd >= 256 || !p->fd[fd].used) {
        return -1;
    }

    strcpy(p->cwd, p->fd[fd].path);

    return 0;
}

static long int syscall_getuid(registers *r) {
    process *p = process_current();
    return p->uid;
}

static long int syscall_getgid(registers *r) {
    process *p = process_current();
    return p->gid;
}

static long int syscall_setuid(registers *r, /*uid_t*/ int32_t uid) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_setgid(registers *r, /*gid_t*/ int32_t gid) {
    process *p = process_current();
    return -1; // Not implemented
}

static long int syscall_geteuid(registers *r) {
    process *p = process_current();
    return p->euid;
}

static long int syscall_getegid(registers *r) {
    process *p = process_current();
    return p->egid;
}

static long int syscall_getppid(registers *r) {
    process *p = process_current();
    return p->ppid;
}

static long int syscall_panic(registers *r, const char *msg) {
    process *p = process_current();

    panic_handler(NULL, (const char *)mmu_get_physical_address_pdt((uintptr_t)msg, p->pdt));
    return -1;
}

static void *syscalls[] = {
    &syscall_read,
    &syscall_write,
    &syscall_open,
    &syscall_close,
    &syscall_stat,
    &syscall_fstat,
    &syscall_lstat,
    &syscall_poll,
    &syscall_lseek,
    &syscall_mmap,
    &syscall_mprotect,
    &syscall_munmap,
    &syscall_brk,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &syscall_pause,
    NULL,
    NULL,
    NULL,
    NULL,
    &syscall_getpid,
    NULL,
    &syscall_socket,
    &syscall_connect,
    &syscall_accept,
    NULL,
    NULL,
    NULL,
    NULL,
    &syscall_shutdown,
    &syscall_bind,
    &syscall_listen,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &syscall_fork,
    NULL,
    &syscall_execve,
    &syscall_exit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &syscall_getuid,
    NULL,
    &syscall_getgid,
    &syscall_setuid,
    &syscall_setgid,
    &syscall_geteuid,
    &syscall_getegid,
    NULL,
    &syscall_getppid,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &syscall_panic,
};

int run_syscall(registers *r) {
    int ret = -1;
    uint32_t no = r->eax;

    uint32_t arg0 = r->ebx;
    uint32_t arg1 = r->ecx;
    uint32_t arg2 = r->edx;
    uint32_t arg3 = r->esi;
    uint32_t arg4 = r->edi;
    uint32_t arg5 = r->ebp;
    dbgprint("syscall: 0x%x(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n", no, arg0, arg1, arg2, arg3, arg4, arg5);

    if (no >= 0 && no < (sizeof(syscalls) / sizeof(syscalls[0])) && syscalls[no]) {
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
