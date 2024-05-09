#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>
#include <stdint.h>
#include "fd.h"
#include "../cpu/mmu.h"
#include "../cpu/system.h"

typedef struct process {
    size_t pid;
    size_t ppid;
    size_t uid;
    size_t gid;
    size_t euid;
    size_t egid;
    uint8_t state;
    bool suspended;
    char cwd[256];
    uint32_t capabilities;
    uintptr_t stack_pointer;
    uintptr_t stack_base;
    uintptr_t entry;
    page_directory_table *pdt;
    registers r;
    size_t fd_count;
    file_descriptor *fd;
} process;

enum ProcessState {
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_STOPPED,
    PROCESS_ZOMBIE
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

process *process_create(uintptr_t entry, uintptr_t stack_base, page_directory_table *pdt, bool switch_to);

void process_save_curr_state(process *p, uint32_t eip);

void process_destroy(process *p);

void process_unload(registers *r);

void process_reload(registers *r);

void process_switch(process *p, registers *r);

void process_enable_round_robin();

void process_disable_round_robin();

void process_round_robin(registers *r);

process *process_by_pid(size_t pid);

process *process_current(void);

process *process_kernel(void);

process *process_fork(process *p);

int process_execve(process *p, registers *r, const char *path, char *const argv[], char *const envp[]);

int process_open_file(process *p, const char *path, int flags);

int process_close_file(process *p, int fd);

int process_stat_file(process *p, const char *path, struct stat *st);

#endif // PROCESS_H
