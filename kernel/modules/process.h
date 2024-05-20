#ifndef PROCESS_H
#define PROCESS_H

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include "fd.h"
#include "sock.h"
#include "vfs.h"
#include "../cpu/mmu.h"
#include "../cpu/system.h"

typedef struct process_t {
    pid_t pid;                 /* process id */
    uid_t uid;                 /* user id */
    gid_t gid;                 /* group id */
    uid_t euid;                /* effective user id */
    gid_t egid;                /* effective group id */
    uint8_t state;             /* process state */
    bool suspended;            /* process is suspended */
    uint8_t priority;          /* process priority */
    size_t preemptions;        /* preemption count */
    int exit_code;             /* process exit code */
    int signal;                /* signal number */
    pid_t waitpid;             /* waitpid */
    int *waitpid_status;       /* waitpid status return pointer */
    mount_t cwd_mount;         /* current working directory mount */
    struct stat cwd;           /* current working directory */
    uint32_t capabilities;     /* process capabilities */
    page_directory_table *pdt; /* page directory table */
    registers r;               /* registers */
    size_t fd_count;           /* file descriptor count */
    file_descriptor *fd;       /* file descriptors */
    size_t sock_count;         /* socket count */
    socket_t *sock;            /* sockets */
    sighandler_t *sig_handles; /* signal handlers */
    struct process_t *parent;  /* parent process */
    struct process_t *next;    /* for linked list */
} process_t;

typedef struct thread_t {
    pid_t tid;                 /* thread id */
    uint8_t state;             /* thread state */
    bool suspended;            /* thread is suspended */
    uint8_t priority;          /* thread priority */
    size_t preemptions;        /* preemption count */
    int exit_code;             /* thread exit code */
    int signal;                /* signal number */
    pid_t waitpid;             /* waitpid */
    int *waitpid_status;       /* waitpid status return pointer */
    mount_t cwd_mount;         /* current working directory mount */
    struct stat cwd;           /* current working directory */
    uint32_t capabilities;     /* thread capabilities */
    registers r;               /* registers */
    sighandler_t *sig_handles; /* signal handlers */
    struct process_t *parent;  /* parent process */
    struct thread_t *next;     /* for linked list */
} thread_t;

enum ProcessState {
    PROCESS_NEW,     /* new process */
    PROCESS_READY,   /* ready to run */
    PROCESS_RUNNING, /* running */
    PROCESS_WAITING, /* waiting for something (semaphore) */
    PROCESS_STOPPED, /* stopped */
    PROCESS_ZOMBIE   /* zombie */
};

enum ProcessPriority {
    PROCESS_PRIO_IDLE = 0,
    PROCESS_PRIO_LOW = 1,
    PROCESS_PRIO_NORMAL = 2,
    PROCESS_PRIO_HIGH = 3,
    PROCESS_PRIO_REALTIME = 4
};

enum ProcessCapability {
    PROCESS_CAP_READ = 1,
    PROCESS_CAP_WRITE = 2,
    PROCESS_CAP_EXEC = 4,
    PROCESS_CAP_CHOWN = 8,
    PROCESS_CAP_CHMOD = 16,
    PROCESS_CAP_KILL = 32,
    PROCESS_CAP_FORK = 64,
    PROCESS_CAP_EXECVE = 128,
    PROCESS_CAP_SETUID = 256,
    PROCESS_CAP_SETGID = 512,
};

void process_init(void);

void process_disable(void);

void process_add_ready_queue(process_t *p);

void process_remove_ready_queue(process_t *p);

process_t *process_create(uintptr_t entry, uintptr_t stack_base, page_directory_table *pdt, bool switch_to);

void process_save_curr_state(process_t *p, uint32_t eip);

void process_destroy(process_t *p, int exit_code);

void process_unload(registers *r);

void process_reload(registers *r);

void process_switch(process_t *p, registers *r);

void process_enable_round_robin();

void process_disable_round_robin();

void process_round_robin(registers *r);

process_t *process_by_pid(size_t pid);

process_t *process_current(void);

bool process_is_idle(void);

void process_idle(registers *r);

void process_notify_waitpid(pid_t pid, int status);

void process_handle_int(process_t *p, registers *r, int int_no);

process_t *process_fork(process_t *p);

int process_execve(process_t *p, registers *r, const char *path, char *const argv[], char *const envp[]);

int process_open_file(process_t *p, const char *path, int flags);

int process_close_file(process_t *p, int fd);

int process_stat_file(process_t *p, const char *path, struct stat *st);

#endif // PROCESS_H
