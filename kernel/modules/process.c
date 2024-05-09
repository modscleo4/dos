#include "process.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"
#include "elf.h"
#include "../debug.h"
#include "../kernel.h"
#include "../rootfs.h"
#include "../cpu/panic.h"
#include "../drivers/serial.h"

struct process_node {
    process p;
    struct process_node *next;
};

static struct {
    struct process_node *first;
    struct process_node *last;
} processes;

static process *current_process = NULL;
static process *kernel_process = NULL;

static struct process_node *process_find_node(process *p) {
    for (struct process_node *node = processes.first; node; node = node->next) {
        if (&node->p == p) {
            return node;
        }
    }

    return NULL;
}

void process_init(void) {
    processes.first = NULL;
    processes.last = NULL;

    dbgprint("Initializing kernel multiprocess\n");

    kernel_process = calloc(1, sizeof(process));
    kernel_process->pid = 0;
    kernel_process->state = PROCESS_RUNNING;
    kernel_process->stack_pointer = 0;
    kernel_process->stack_base = 0;
    kernel_process->entry = 0xc0100000;
    kernel_process->pdt = current_pdt;

    current_process = kernel_process;

    process_save_curr_state(kernel_process, (uint32_t)&kernel_int_wait);
}

void process_disable(void) {
    dbgprint("Disabling multiprocess\n");
    current_process = kernel_process;
    process_disable_round_robin();
}

inline void process_save_curr_state(process *p, uint32_t eip) {
    asm volatile("mov %%esp, %0" : "=r"(p->stack_pointer));
    asm volatile("mov %%ebp, %0" : "=r"(p->stack_base));
    asm volatile("mov %%cs, %0" : "=r"(p->r.cs));
    asm volatile("mov %%ds, %0" : "=r"(p->r.ds));
    asm volatile("mov %%es, %0" : "=r"(p->r.es));
    asm volatile("mov %%fs, %0" : "=r"(p->r.fs));
    asm volatile("mov %%gs, %0" : "=r"(p->r.gs));
    asm volatile("mov %%ss, %0" : "=r"(p->r.ss));
    asm volatile("pushf; pop %0" : "=r"(p->r.eflags));
    asm volatile("mov %%esp, %0" : "=r"(p->r.useresp));
    asm volatile("mov %%ebp, %0" : "=r"(p->r.ebp));
    asm volatile("mov %%eax, %0" : "=r"(p->r.eax));
    asm volatile("mov %%ebx, %0" : "=r"(p->r.ebx));
    asm volatile("mov %%ecx, %0" : "=r"(p->r.ecx));
    asm volatile("mov %%edx, %0" : "=r"(p->r.edx));
    asm volatile("mov %%esi, %0" : "=r"(p->r.esi));
    asm volatile("mov %%edi, %0" : "=r"(p->r.edi));
    //asm volatile("mov %%eip, %0" : "=r"(p->r.eip));
    p->r.eip = eip;
    dbgprint("Saving processor state\n");
}

process *process_create(uintptr_t entry, uintptr_t stack_base, page_directory_table *pdt, bool switch_to) {
    size_t pid = 1;
    if (processes.last) {
        pid = processes.last->p.pid + 1;
    }

    dbgprint("Creating process with PID %d, entry: 0x%x, stack: 0x%x, pdt: 0x%x\n", pid, entry, stack_base, pdt);

    struct process_node *node = calloc(1, sizeof(struct process_node));
    node->p.pid = pid;
    node->p.state = PROCESS_RUNNING;
    node->p.stack_pointer = stack_base;
    node->p.stack_base = stack_base;
    node->p.entry = entry;
    node->p.pdt = pdt;
    node->p.fd_count = 256;
    node->p.fd = calloc(node->p.fd_count, sizeof(file_descriptor));

    if (switch_to) {
        current_process = &node->p;
    }

    if (!processes.first) {
        processes.first = node;
        processes.last = node;
    } else {
        processes.last->next = node;
        processes.last = node;
    }

    return &node->p;
}

void process_destroy(process *p) {
    if (p == kernel_process) {
        return; // DO NOT destroy the kernel process
    }

    dbgprint("Destroying process %d\n", p->pid);
    p->state = PROCESS_ZOMBIE;
    //p->stack_pointer = 0;
    //p->stack_base = 0;
    //p->entry = 0;
    //p->pdt = NULL;
    mmu_free_pages(p->pdt, 0, (0xc0000000 >> 22) * 1024);
    bitmap_free_pages(p->pdt, 1);

    for (int i = 0; i < p->fd_count; i++) {
        if (p->fd[i].used) {
            p->fd[i].used = false;
            if (p->fd[i].type == S_IFREG) {
                //
            }
        }
    }

    free(p->fd);

    // Update parent process children
    for (struct process_node *node = processes.first; node; node = node->next) {
        if (node->p.ppid == p->pid) {
            node->p.ppid = p->ppid;
        }
    }

    // Fix the process list
    struct process_node *prev = NULL;
    for (struct process_node *node = processes.first; node; node = node->next) {
        if (&node->p == p) {
            if (prev) {
                prev->next = node->next;
            } else {
                processes.first = node->next;
            }

            if (processes.last == node) {
                processes.last = prev;
            }

            free(node);
            break;
        }

        prev = node;
    }
}

void process_unload(registers *r) {
    if (current_process == kernel_process) { // Don't unload the kernel process
        return;
    }

    mmu_load_kernel_pdt();

    process *p = process_current();

    dbgprint("Unloading process %d\n", current_process->pid);

    //p->r = *r;
    memcpy(&p->r, r, sizeof(registers));
    p->stack_pointer = r->useresp;
    p->state = PROCESS_WAITING;
}

void process_reload(registers *r) {
    if (current_process == kernel_process) { // Don't reload the kernel process
        return;
    }

    process *p = process_current();

    if (p->state == PROCESS_RUNNING) {
        return;
    }

    dbgprint("Reloading process %d after iret\n", current_process->pid);

    //*r = p->r;
    memcpy(r, &p->r, sizeof(registers));
    r->useresp = p->stack_pointer;
    p->state = PROCESS_RUNNING;

    mmu_load_pdt(p->pdt);
}

void process_switch(process *p, registers *r) {
    if (current_process != p) {
        dbgprint("Switching to process %d\n", p->pid);
        current_process = p;
        //*r = p->r;
        memcpy(r, &p->r, sizeof(registers));
        p->state = PROCESS_RUNNING;

        mmu_load_pdt(p->pdt);
    }
}

static bool round_robin = false;

void process_enable_round_robin(void) {
    round_robin = true;
}

void process_disable_round_robin(void) {
    round_robin = false;
}

static void process_do_round_robin(process *p, registers *r) {
    if (!p) {
        return;
    }

    dbgprint("Round robin: switching to process %d\n", p->pid);
    if (current_process != kernel_process) {
        process_unload(r);
    }

    process_switch(p, r);
}

void process_round_robin(registers *r) {
    if (!round_robin) {
        return;
    }

    struct process_node *node = process_find_node(current_process);
    if (node) {
        node = node->next;
        while (node) {
            if (!node->p.suspended) {
                process_do_round_robin(&node->p, r);
                return;
            }

            node = node->next;
        }
    }

    node = processes.first;
    while (node && &node->p != current_process) {
        if (!node->p.suspended) {
            process_do_round_robin(&node->p, r);
            return;
        }

        node = node->next;
    }
}

process *process_by_pid(size_t pid) {
    for (struct process_node *node = processes.first; node; node = node->next) {
        if (node->p.pid == pid) {
            return &node->p;
        }
    }

    return NULL;
}

process *process_current(void) {
    return current_process;
}

process *process_kernel(void) {
    return kernel_process;
}

process *process_fork(process *p) {
    dbgprint("Forking process %d\n", p->pid);
    page_directory_table *pdt = mmu_clone_pdt(p->pdt);
    process *child = process_create(p->entry, p->stack_base, pdt, false);
    if (child) {
        child->ppid = p->pid;
        child->uid = p->uid;
        child->gid = p->gid;
        child->euid = p->euid;
        child->egid = p->egid;
        child->stack_pointer = p->stack_pointer;
        memcpy(child->cwd, p->cwd, 256);
        memcpy(&child->r, &p->r, sizeof(registers));
        memcpy(child->fd, p->fd, sizeof(file_descriptor) * 256);
        return child;
    }

    return NULL;
}

int process_execve(process *p, registers *r, const char *path, char *const argv[], char *const envp[]) {
    dbgprint("Replacing process %d with new executable %s\n", p->pid, path);
    struct stat st;
    int stat_ret = 0;
    if ((stat_ret = rootfs.stat(&rootfs_io, &rootfs, path, &st))) {
        return stat_ret;
    }

    size_t elf_file_size = st.st_size;
    dbgprint("File size: %d bytes\n", elf_file_size);

    void *addr;
    if (!(addr = rootfs.load_file(&rootfs_io, &rootfs, &st))) {
        dbgprint("Could not allocate or load file.\n");
        return -ENOMEM;
    }

    dbgprint("%s loaded at address 0x%x\n", path, addr);

    elf_header_ident *header = (elf_header_ident *)addr;
    if (memcmp(header->magic, elf_magic, 4)) {
        dbgprint("Not an ELF file\n");
        return -ENOEXEC;
    }

    if (header->version == ELF_ARCH_X86) {
        elf32_header *exec_header = (elf32_header *)addr;

        dbgprint("x86 ELF file (%d bytes)\n", elf_file_size);
        dbgprint("Entry point: %x\n", exec_header->entry);

        page_directory_table *process_page_directory = mmu_new_page_directory();
        void *process_stack = bitmap_alloc_page();

        elf32_program_header *program_header = (elf32_program_header *)(((void *)exec_header) + exec_header->program_header_offset);
        for (int i = 1; i <= exec_header->program_header_count; i++) {
            dbgprint("Program header %d: type=%d, offset=%x, virtual_address=%x, physical_address=%x, file_size=%d, memory_size=%d\n", i, program_header->type, program_header->offset, program_header->virtual_address, program_header->physical_address, program_header->file_size, program_header->memory_size);
            if (program_header->type == ELF_PT_LOAD) { // map to virtual address
                mmu_map_pages(process_page_directory, mmu_get_physical_address((uintptr_t)addr + program_header->offset), program_header->virtual_address, program_header->memory_size / BITMAP_PAGE_SIZE + 1, true, true, true);
            }

            program_header++;
        }

        // Map the ELF stack to 4 kiB below the kernel
        mmu_map_pages(process_page_directory, (uintptr_t)process_stack, (uintptr_t)0xc0000000 - 0x1000, 1, true, true, true);

        void *a = bitmap_alloc_page();
        mmu_map_pages(process_page_directory, (uintptr_t)a, (uintptr_t)0xb00000, 1, true, true, true);

        // Map the kernel to 0xc0000000 (higher half)
        mmu_copy_kernel_pages(process_page_directory);

        mmu_free_pages(p->pdt, 0, (0xc0000000 >> 22) * 1024);
        bitmap_free_pages(p->pdt, 1);

        p->entry = (uintptr_t)exec_header->entry;
        p->stack_base = (uintptr_t)0xc0000000 - 0x4;
        p->stack_pointer = (uintptr_t)0xc0000000 - 0x12;
        p->pdt = process_page_directory;

        p->r.eip = p->entry;
        p->r.useresp = p->stack_pointer;

        return 0;
    } else if (header->version == ELF_ARCH_X86_64) {
        elf64_header *exec_header = (elf64_header *)addr;

        dbgprint("x86_64 ELF file\n");
        dbgprint("Entry point: %x\n", exec_header->entry);

        page_directory_table *process_page_directory = mmu_new_page_directory();
        void *process_stack = bitmap_alloc_page();

        elf64_program_header *program_header = (elf64_program_header *)(((void *)exec_header) + exec_header->program_header_offset);
        for (int i = 1; i <= exec_header->program_header_count; i++) {
            if (program_header->type == ELF_PT_LOAD) { // map to virtual address
                mmu_map_pages(process_page_directory, mmu_get_physical_address((uintptr_t)addr + program_header->offset), program_header->virtual_address, program_header->memory_size / BITMAP_PAGE_SIZE + 1, true, true, true);
            }

            program_header++;
        }

        // Map the ELF stack to 8 kiB below the kernel
        mmu_map_pages(process_page_directory, (uintptr_t)process_stack, (uintptr_t)0xc0000000 - 0x2000, 2, true, true, true);

        // Map the kernel to 0xc0000000 (higher half)
        mmu_copy_kernel_pages(process_page_directory);

        mmu_free_pages(p->pdt, 0, (0xc0000000 >> 22) * 1024);
        bitmap_free_pages(p->pdt, 1);

        p->entry = (uintptr_t)exec_header->entry;
        p->stack_base = (uintptr_t)0xc0000000 - 0x8;
        p->stack_pointer = (uintptr_t)0xc0000000 - 0x24;
        p->pdt = process_page_directory;

        p->r.eip = p->entry;
        p->r.useresp = p->stack_pointer;

        return 0;
    } else {
        dbgprint("Unsupported architecture: %x\n", header->version);
        return -ENOEXEC;
    }
}

int process_open_file(process *p, const char *path, int flags) {
    for (int i = 0; i < p->fd_count; i++) {
        if (!p->fd[i].used) {
            p->fd[i].used = true;
            p->fd[i].flags = flags;
            p->fd[i].offset = 0;
            strncpy(p->fd[i].path, path, 256);

            if ((flags & 0x3) == O_RDONLY) {
                p->fd[i].access.read = true;
            } else if ((flags & 0x3) == O_WRONLY) {
                p->fd[i].access.write = true;
            } else if ((flags & 0x3) == O_RDWR) {
                p->fd[i].access.read = true;
                p->fd[i].access.write = true;
            }

            if (strncmp(path, "/dev/tty", 8) == 0) {
                dbgprint("Opening TTY %d\n", atoi(path + 8));
                p->fd[i].type = S_IFCHR;
                p->fd[i].tty = atoi(path + 8);
                p->fd[i].tty_canon = true;

                if (p->fd[i].tty == 0) {
                    p->fd[i].tty = 1;
                } else if (p->fd[i].tty > 9) {
                    return -ENOENT;
                }
            } else {
                dbgprint("Opening file %s\n", path);
                p->fd[i].type = S_IFREG;
                p->fd[i].io = &rootfs_io;
                p->fd[i].fs = &rootfs;
                p->fd[i].offset = 0;
                int stat_ret = 0;
                if ((stat_ret = rootfs.stat(&rootfs_io, &rootfs, path, &p->fd[i].st))) {
                    p->fd[i].used = false;
                    return stat_ret;
                }
            }

            return i;
        }
    }

    return -ENOMEM;
}

int process_stat_file(process *p, const char *path, struct stat *st) {
    return rootfs.stat(&rootfs_io, &rootfs, path, st);
}

int process_close_file(process *p, int fd) {
    if (fd < p->fd_count) {
        if (p->fd[fd].used) {
            p->fd[fd].used = false;
            if (p->fd[fd].type == S_IFREG) {
                //
            }

            return 0;
        }
    }

    return -EBADFD;
}

void process_set_by_pid(size_t pid) {
    if (pid < 0) {
        return;
    }

    process *p = process_by_pid(pid);
    if (!p) {
        return;
    }

    current_process = p;
}
