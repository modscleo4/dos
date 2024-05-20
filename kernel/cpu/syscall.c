#include "syscall.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "../modules/task.h"
#include "../modules/net/dns.h"
#include "../modules/net/tcp.h"
#include "../modules/net/udp.h"

// This var is to store the current SYSCALL return value
int32_t __syscall_ret;

static inline void syscall_sleep_proc(registers *r, process_t *p) {
    process_idle(r);
    p->suspended = true;
    process_disable_round_robin();
    interrupts_reenable();
}

static inline void syscall_wake_proc(registers *r, process_t *p) {
    interrupts_disable();
    process_enable_round_robin();
    process_switch(p, r);
    p->suspended = false;
}

static void syscall_on_idle(registers *r, int32_t(*callback)(process_t *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)) {
    process_t *p = process_current();
    process_idle(r);
    p->suspended = true;

    task_t *t = malloc(sizeof(task_t));
    t->process = p;
    t->type = TASK_TYPE_IO;
    t->callback = callback;
    task_push(t);
}

static int32_t syscall_read(registers *r, int fd, void *buf, size_t count) {
    process_t *p = process_current();

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

static int32_t syscall_write(registers *r, int fd, void *buf, size_t count) {
    process_t *p = process_current();

    // Switch to ring0, because write() may have to wait for data
    syscall_sleep_proc(r, p);

    int ret = write(
        &p->fd[fd],
        (void *)mmu_get_physical_address_pdt((uintptr_t)buf, p->pdt),
        count
    );

    if (ret < 0) {
        dbgprint("write failed: %d\n", ret);
    } else {
        dbgprint("written %d bytes\n", ret);
    }

    syscall_wake_proc(r, p);

    return ret;
}

static int32_t syscall_open(registers *r, char *path, int flags) {
    process_t *p = process_current();

    // Switch to ring0, because open() may have to wait for data
    syscall_sleep_proc(r, p);

    size_t path_len = mmu_strlen(p->pdt, (uintptr_t)path);
    char *k_path = (char *)malloc(path_len + 1);
    mmu_strcpy(p->pdt, (uintptr_t)path, k_path);

    int fd = process_open_file(
        p,
        k_path,
        flags
    );

    free(k_path);

    syscall_wake_proc(r, p);
    return fd;
}

static int32_t syscall_close(registers *r, int fd) {
    process_t *p = process_current();
    return process_close_file(p, fd);
}

static int32_t syscall_stat(registers *r, const char *filename, struct stat *statbuf) {
    process_t *p = process_current();

    // Switch to ring0, because stat() may have to wait for data
    syscall_sleep_proc(r, p);

    size_t filename_len = mmu_strlen(p->pdt, (uintptr_t)filename);
    char *k_filename = (char *)malloc(filename_len + 1);
    mmu_strcpy(p->pdt, (uintptr_t)filename, k_filename);

    int ret = process_stat_file(
        p,
        k_filename,
        statbuf ? (struct stat *)mmu_get_physical_address_pdt((uintptr_t)statbuf, p->pdt) : NULL
    );

    free(k_filename);

    syscall_wake_proc(r, p);
    return ret;
}

static int32_t syscall_fstat(registers *r, unsigned int fd, struct stat *statbuf) {
    process_t *p = process_current();

    if (fd < 0 || fd >= p->fd_count || !p->fd[fd].used) {
        return -EBADFD;
    }

    file_descriptor *f = &p->fd[fd];
    if (statbuf) {
        memcpy((struct stat *)mmu_get_physical_address_pdt((uintptr_t)statbuf, p->pdt), &f->st, sizeof(struct stat) - sizeof(f->st.st_private));
    }

    return 0;
}

static int32_t syscall_lstat(registers *r, const char *filename, struct stat *statbuf) {
    process_t *p = process_current();

    // Switch to ring0, because stat() may have to wait for data
    syscall_sleep_proc(r, p);

    size_t filename_len = mmu_strlen(p->pdt, (uintptr_t)filename);
    char *k_filename = (char *)malloc(filename_len + 1);
    mmu_strcpy(p->pdt, (uintptr_t)filename, k_filename);

    int ret = process_stat_file(
        p,
        k_filename,
        statbuf ? (struct stat *)mmu_get_physical_address_pdt((uintptr_t)statbuf, p->pdt) : NULL
    );

    free(k_filename);

    syscall_wake_proc(r, p);
    return ret;
}

static int32_t syscall_poll(registers *r, /*struct pollfd*/ void *fds, unsigned long nfds, int timeout) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_lseek(registers *r, unsigned int fd, /*off_t*/ int32_t offset, int whence) {
    process_t *p = process_current();

    if (fd < 0 || fd >= p->fd_count || !p->fd[fd].used) {
        return -EBADFD;
    }

    file_descriptor *f = &p->fd[fd];

    switch (whence) {
        case SEEK_SET:
            f->offset = offset;
            break;
        case SEEK_CUR:
            f->offset += offset;
            break;
        case SEEK_END:
            f->offset = f->st.st_size + offset;
            break;
        default:
            return -EINVAL;
    }

    return f->offset;
}

static int32_t syscall_mmap(registers *r, void *addr, size_t length, int prot, int flags, int fd, long offset) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_mprotect(registers *r, void *addr, size_t length, int prot) {
    process_t *p = process_current();
    return -1; // Not implemented
}


static int32_t syscall_munmap(registers *r, void *addr, size_t length) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_brk(registers *r, void *addr) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_signal(registers *r, int signum, sighandler_t handler) {
    process_t *p = process_current();

    if (signum < 0 || signum >= NSIG) {
        return -EINVAL;
    }

    if (signum == 0) {
        return 0;
    }

    p->sig_handles[signum] = handler;

    return (int32_t) handler;
}

static int32_t syscall_pause(registers *r) {
    process_t *p = process_current();
    process_idle(r);
    return -1; // Not implemented
}

static int32_t syscall_getpid(registers *r) {
    process_t *p = process_current();
    return p->pid;
}

static int32_t syscall_socket(registers *r, int domain, int type, int protocol) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_connect(registers *r, int sockfd, const /*struct sockaddr*/ void *addr, /*socklen_t*/ size_t addrlen) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_accept(registers *r, int sockfd, /*struct sockaddr*/ void * restrict addr, /*socklen_t*/ void * restrict addrlen) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_shutdown(registers *r, int sockfd, int how) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_bind(registers *r, int sockfd, const /*struct sockaddr*/ void *addr, /*socklen_t*/ size_t addrlen) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_listen(registers *r, int sockfd, int backlog) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_fork(registers *r) {
    process_t *p = process_current();
    process_t *child = process_fork(p);
    if (!child) {
        return -1;
    }

    // Child process returns 0
    child->r.eax = 0;

    return child->pid;
}

static int32_t syscall_execve(registers *r, const char *filename, char *const argv[], char *const envp[]) {
    process_t *p = process_current();

    syscall_sleep_proc(r, p);

    size_t filename_len = mmu_strlen(p->pdt, (uintptr_t)filename);
    char *k_filename = (char *)malloc(filename_len + 1);
    mmu_strcpy(p->pdt, (uintptr_t)filename, k_filename);

    int ret = process_execve(
        p,
        r,
        k_filename,
        argv,
        envp
    );

    free(k_filename);

    syscall_wake_proc(r, p);

    return ret;
}

static int32_t syscall_exit(registers *r, int exit_code) {
    process_t *p = process_current();
    process_destroy(p, exit_code);

    process_idle(r);

    process_notify_waitpid(p->pid, exit_code);

    return exit_code;
}

static int32_t syscall_waitpid(registers *r, int pid, int *status, int options) {
    process_t *p = process_current();

    process_t *target = process_by_pid(pid);
    if (!target) {
        return -ESRCH;
    }

    if (target->state == PROCESS_ZOMBIE) {
        mmu_writel(p->pdt, (uintptr_t)status, target->exit_code);
        return 0;
    }

    dbgprint("Making process %d wait for %d\n", p->pid, pid);
    p->waitpid = pid;

    process_idle(r);
    p->suspended = true;
    p->waitpid_status = status;
    return 0;
}

static int32_t syscall_kill(registers *r, int pid, int sig) {
    process_t *p = process_current();

    if (pid == 0) {
        pid = p->pid;
    }

    process_t *target = process_by_pid(pid);
    if (!target) {
        return -ESRCH;
    }

    if (sig < 0 || sig >= NSIG) {
        return -EINVAL;
    }

    if (sig == 0) {
        return 0;
    }

    process_notify_waitpid(pid, sig << 16);

    if (sig == SIGKILL) {
        process_destroy(target, sig);
        if (target == p) {
            process_idle(r);
        }

        if (target->pid == 1) {
            panic("init process killed");
        }

        return 0;
    }

    if (target->signal != 0) {
        return -EBUSY;
    }

    target->signal = sig;

    return 0;
}

static int32_t syscall_getcwd(registers *r, char *buf, size_t size) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_chdir(registers *r, const char *path) {
    process_t *p = process_current();

    syscall_sleep_proc(r, p);

    int ret = process_stat_file(p, (const char *)mmu_get_physical_address_pdt((uintptr_t)path, p->pdt), &p->cwd);

    syscall_wake_proc(r, p);

    return ret;
}

static int32_t syscall_fchdir(registers *r, int fd) {
    process_t *p = process_current();

    if (fd < 0 || fd >= 256 || !p->fd[fd].used) {
        return -1;
    }

    memcpy(&p->cwd, &p->fd[fd].st, sizeof(struct stat));

    return 0;
}

static int32_t syscall_getuid(registers *r) {
    process_t *p = process_current();
    return p->uid;
}

static int32_t syscall_getgid(registers *r) {
    process_t *p = process_current();
    return p->gid;
}

static int32_t syscall_setuid(registers *r, /*uid_t*/ int32_t uid) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_setgid(registers *r, /*gid_t*/ int32_t gid) {
    process_t *p = process_current();
    return -1; // Not implemented
}

static int32_t syscall_geteuid(registers *r) {
    process_t *p = process_current();
    return p->euid;
}

static int32_t syscall_getegid(registers *r) {
    process_t *p = process_current();
    return p->egid;
}

static int32_t syscall_getppid(registers *r) {
    process_t *p = process_current();
    return p->parent ? p->parent->pid : 0;
}

static int32_t syscall_panic(registers *r, const char *msg) {
    process_t *p = process_current();

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
    &syscall_signal,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
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
    &syscall_waitpid,
    &syscall_kill,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
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
        ret = -ENOSYS;
    }

    return ret;
}

void syscall_init(void) {
    idt_set_gate(128, (uint32_t)syscall_handler, 0x08, 0x8E);
}
