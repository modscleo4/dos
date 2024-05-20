#include "process.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"
#include "elf.h"
#include "task.h"
#include "vfs/proc.h"
#include "../bits.h"
#include "../debug.h"
#include "../kernel.h"
#include "../rootfs.h"
#include "../cpu/panic.h"
#include "../drivers/serial.h"

pid_t process_next_pid = 1;

static process_t *ready_queue = NULL;
static process_t *current_process = NULL;
static process_t *kernel_idle_process = NULL;
static bool round_robin = false;

void process_init(void) {
    dbgprint("Initializing kernel multiprocess\n");

    kernel_idle_process = calloc(1, sizeof(process_t));
    kernel_idle_process->pid = 0;
    kernel_idle_process->state = PROCESS_RUNNING;
    kernel_idle_process->pdt = current_pdt;

    current_process = kernel_idle_process;

    process_save_curr_state(kernel_idle_process, (uint32_t)&kernel_int_wait);
}

void process_disable(void) {
    dbgprint("Disabling multiprocess\n");
    current_process = kernel_idle_process;
    process_disable_round_robin();
}

inline void process_save_curr_state(process_t *p, uint32_t eip) {
    asm volatile("mov %%esp, %0" : "=r"(p->r.esp));
    //asm volatile("mov %%ebp, %0" : "=r"(p->r.ebp));
    p->r.ebp = 0;
    asm volatile("mov %%cs, %0" : "=r"(p->r.cs));
    asm volatile("mov %%ds, %0" : "=r"(p->r.ds));
    asm volatile("mov %%es, %0" : "=r"(p->r.es));
    asm volatile("mov %%fs, %0" : "=r"(p->r.fs));
    asm volatile("mov %%gs, %0" : "=r"(p->r.gs));
    asm volatile("mov %%ss, %0" : "=r"(p->r.ss));
    asm volatile("pushf; pop %0" : "=r"(p->r.eflags));
    asm volatile("mov %%esp, %0" : "=r"(p->r.useresp));
    asm volatile("mov %%eax, %0" : "=r"(p->r.eax));
    asm volatile("mov %%ebx, %0" : "=r"(p->r.ebx));
    asm volatile("mov %%ecx, %0" : "=r"(p->r.ecx));
    asm volatile("mov %%edx, %0" : "=r"(p->r.edx));
    asm volatile("mov %%esi, %0" : "=r"(p->r.esi));
    asm volatile("mov %%edi, %0" : "=r"(p->r.edi));
    // asm volatile("mov %%eip, %0" : "=r"(p->r.eip));
    p->r.eip = eip;
    dbgprint("Saving processor state\n");
}

void process_add_ready_queue(process_t *p) {
    if (!ready_queue) {
        ready_queue = p;
    } else {
        process_t *last = ready_queue;
        while (last->next) {
            last = last->next;
        }

        last->next = p;
    }
}

void process_remove_ready_queue(process_t *p) {
    process_t *prev = NULL;
    for (process_t *node = ready_queue; node; node = node->next) {
        if (node == p) {
            if (prev) {
                prev->next = node->next;
            } else {
                // p is the first node
                ready_queue = node->next;
            }

            break;
        }

        prev = node;
    }
}

static inline void process_fill_sig_handlers(process_t *p) {
    for (int i = 0; i < NSIG; i++) {
        p->sig_handles[i] = SIG_DFL;
    }
}

process_t *process_create(uintptr_t entry, uintptr_t stack_base, page_directory_table *pdt, bool switch_to) {
    size_t pid = process_next_pid++;

    dbgprint("Creating process with PID %d, entry: 0x%x, stack: 0x%x, pdt: 0x%x\n", pid, entry, stack_base, pdt);

    process_t *node = calloc(1, sizeof(process_t));
    node->pid = pid;
    node->state = PROCESS_READY;
    node->pdt = pdt;
    node->fd_count = 256;
    node->fd = calloc(node->fd_count, sizeof(file_descriptor));
    node->sig_handles = calloc(NSIG, sizeof(sighandler_t));
    process_fill_sig_handlers(node);

    node->r.eip = entry;
    node->r.useresp = stack_base;
    node->r.ebp = stack_base;

    vfs_stat("/", root_mount, &root_mount->rootdir.st, NULL, &node->cwd);

    if (switch_to) {
        current_process = node;
    }

    process_add_ready_queue(node);

    return node;
}

void process_destroy(process_t *p, int exit_code) {
    if (p == kernel_idle_process) {
        return; // DO NOT destroy the kernel process
    }

    dbgprint("Destroying process %d\n", p->pid);
    p->state = PROCESS_ZOMBIE;
    p->exit_code = exit_code;
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
    for (process_t *node = ready_queue; node; node = node->next) {
        if (node->parent == p) {
            node->parent = p->parent;
        }
    }

    // Fix the process list
    process_remove_ready_queue(p);

    task_remove_all_from(p);

    free(p);
}

void process_unload(registers *r) {
    if (!round_robin) { // Don't unload the kernel process
        return;
    }

    mmu_load_kernel_pdt();

    process_t *p = process_current();

    dbgprint("Unloading process %d\n", current_process->pid);

    // p->r = *r;
    memcpy(&p->r, r, sizeof(registers));
    p->state = PROCESS_READY;
    p->preemptions++;
}

void process_reload(registers *r) {
    if (!round_robin) { // Don't reload the kernel process
        return;
    }

    process_t *p = process_current();

    if (p->state == PROCESS_RUNNING) {
        return;
    }

    dbgprint("Reloading process %d after iret\n", current_process->pid);
    current_process->state = PROCESS_READY;

    //*r = p->r;
    memcpy(r, &p->r, sizeof(registers));
    p->state = PROCESS_RUNNING;

    mmu_load_pdt(p->pdt);
}

void process_switch(process_t *p, registers *r) {
    if (current_process != p) {
        dbgprint("Switching to process %d\n", p->pid);
        current_process->state = PROCESS_READY;
        current_process = p;
        if (p->signal != 0) {
            dbgprint("Process %d has pending signal %d\n", p->pid, p->signal);
            sighandler_t handler = p->sig_handles[p->signal];
            if (handler && handler != SIG_IGN && handler != SIG_DFL && handler != SIG_HOLD) {
                uint32_t eip = p->r.eip;
                uint32_t esp = p->r.useresp;
                p->r.eip = (uint32_t)handler;
                p->r.useresp -= 8;

                mmu_writel(p->pdt, esp - 4, p->signal);
                mmu_writel(p->pdt, esp - 8, eip);
            }
        }
        //*r = p->r;
        memcpy(r, &p->r, sizeof(registers));
        p->state = PROCESS_RUNNING;

        mmu_load_pdt(p->pdt);
    }
}

void process_enable_round_robin(void) {
    round_robin = true;
}

void process_disable_round_robin(void) {
    round_robin = false;
}

static void process_do_round_robin(process_t *p, registers *r) {
    if (!p) {
        return;
    }

    dbgprint("Round robin: switching to process %d\n", p->pid);
    if (current_process != kernel_idle_process) {
        process_unload(r);
    }

    process_switch(p, r);
}

void process_round_robin(registers *r) {
    if (!round_robin) {
        return;
    }

    process_t *node = current_process;
    if (node && node != kernel_idle_process) {
        node = node->next;
        while (node) {
            if (!node->suspended && node->state == PROCESS_READY) {
                process_do_round_robin(node, r);
                return;
            }

            node = node->next;
        }
    }

    node = ready_queue;
    while (node && node != current_process) {
        if (!node->suspended && node->state == PROCESS_READY) {
            process_do_round_robin(node, r);
            return;
        }

        node = node->next;
    }
}

process_t *process_by_pid(size_t pid) {
    for (process_t *node = ready_queue; node; node = node->next) {
        if (node->pid == pid) {
            return node;
        }
    }

    return NULL;
}

process_t *process_current(void) {
    return current_process;
}

inline bool process_is_idle(void) {
    return current_process == kernel_idle_process;
}

void process_idle(registers *r) {
    process_switch(kernel_idle_process, r);
}

void process_notify_waitpid(pid_t pid, int status) {
    for (process_t *node = ready_queue; node; node = node->next) {
        if (node->waitpid == pid) {
            // Found a process waiting for this pid
            node->waitpid = 0;
            node->suspended = false;
            node->r.eax = pid;

            if (node->waitpid_status) {
                mmu_writel(node->pdt, (uintptr_t)node->waitpid_status, status);
            }
        }
    }
}

void process_handle_int(process_t *p, registers *r, int int_no) {
    process_destroy(p, 1);
    process_idle(r);
}

process_t *process_fork(process_t *p) {
    dbgprint("Forking process %d\n", p->pid);
    page_directory_table *pdt = mmu_clone_pdt(p->pdt);
    process_t *child = process_create(p->r.eip, p->r.useresp, pdt, false);
    if (!child) {
        return NULL;
    }

    child->parent = p;
    child->uid = p->uid;
    child->gid = p->gid;
    child->euid = p->euid;
    child->egid = p->egid;
    memcpy(&child->cwd, &p->cwd, sizeof(struct stat));
    memcpy(&child->r, &p->r, sizeof(registers));
    memcpy(child->fd, p->fd, sizeof(file_descriptor) * 256);
    return child;
}

static int serial_write_fn(const char *str, ...) {
    va_list args;
    va_start(args, str);
    serial_write_str_varargs(SERIAL_COM1, str, args);
    va_end(args);

    return 0;
}

int process_execve(process_t *p, registers *r, const char *path, char *const argv[], char *const envp[]) {
    dbgprint("Replacing process %d with new executable at %s\n", p->pid, path);
    mount_t mount;
    struct stat st;
    int stat_ret = 0;
    if ((stat_ret = vfs_stat(path, &p->cwd_mount, &p->cwd, &mount, &st))) {
        dbgprint("Failed to stat %s\n", path);
        return stat_ret;
    }

    size_t elf_file_size = st.st_size;
    dbgprint("File size: %d bytes\n", elf_file_size);

    void *addr;
    if (!(addr = vfs_load_file(&mount, &st))) {
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
        uint32_t *process_stack = (uint32_t *)bitmap_alloc_contiguous_pages(2);

        elf32_program_header *program_header = (elf32_program_header *)(((uint8_t *)exec_header) + exec_header->program_header_offset);
        for (int i = 1; i <= exec_header->program_header_count; i++) {
            dbgprint("Program header %d: type=%d, offset=%x, virtual_address=%x, physical_address=%x, file_size=%d, memory_size=%d\n", i, program_header->type, program_header->offset, program_header->virtual_address, program_header->physical_address, program_header->file_size, program_header->memory_size);
            if (program_header->type == ELF_PT_LOAD) { // map to virtual address
                mmu_map_pages(process_page_directory, mmu_get_physical_address((uintptr_t)addr + program_header->offset), program_header->virtual_address, program_header->memory_size / BITMAP_PAGE_SIZE + 1, true, true, true);
            }

            program_header++;
        }

        // Map the ELF stack to 4 kiB below the kernel
        mmu_map_pages(process_page_directory, (uintptr_t)process_stack, (uintptr_t)0xc0000000 - 0x2000, 2, true, true, false);

        // Map 1024 pages for the heap
        void *heap = bitmap_alloc_contiguous_pages(1024);
        mmu_map_pages(process_page_directory, (uintptr_t)heap, (uintptr_t)0xc00000, 1024, true, true, false);

        uintptr_t *process_argv = bitmap_alloc_page();
        char *process_argv_str = bitmap_alloc_page();
        mmu_map_pages(process_page_directory, (uintptr_t)process_argv, (uintptr_t)0xb00000, 1, true, true, false);
        mmu_map_pages(process_page_directory, (uintptr_t)process_argv_str, (uintptr_t)0xb01000, 1, true, true, false);
        memset(process_argv, 0, BITMAP_PAGE_SIZE);
        memset(process_argv_str, 0, BITMAP_PAGE_SIZE);

        int argc = 0;
        uintptr_t argv_next = 0;
        uint32_t argv_addr;
        while ((argv_addr = mmu_readl(p->pdt, (uintptr_t)argv + 4 * argc))) {
            process_argv[argc] = 0xb01000 + argv_next;
            size_t argv_len = mmu_strlen(p->pdt, argv_addr);
            if (!argv_len) {
                break;
            }

            dbgprint("Process argv[%d]: %p (len=%d), copying to %p...\n", argc, argv_addr, argv_len, &process_argv[argv_next]);
            mmu_strncpy(p->pdt, argv_addr, &process_argv_str[argv_next], argv_len);
            argv_next += argv_len + 1;
            argc++;
        }

        uintptr_t *process_envp = bitmap_alloc_page();
        char *process_envp_str = bitmap_alloc_page();
        mmu_map_pages(process_page_directory, (uintptr_t)process_envp, (uintptr_t)0xb02000, 1, true, true, false);
        mmu_map_pages(process_page_directory, (uintptr_t)process_envp_str, (uintptr_t)0xb03000, 1, true, true, false);
        memset(process_envp, 0, BITMAP_PAGE_SIZE);
        memset(process_envp_str, 0, BITMAP_PAGE_SIZE);

        int envc = 0;
        uintptr_t envp_next = 0;
        uint32_t envp_addr;
        while ((envp_addr = mmu_readl(p->pdt, (uintptr_t)envp + 4 * envc))) {
            process_envp[envc] = 0xb03000 + envp_next;
            size_t envp_len = mmu_strlen(p->pdt, envp_addr);
            if (!envp_len) {
                break;
            }

            dbgprint("Process envp[%d]: %p (len=%d), copying to %p...\n", envc, envp_addr, envp_len, &process_envp[envp_next]);
            mmu_strncpy(p->pdt, envp_addr, &process_envp_str[envp_next], envp_len);
            envp_next += envp_len + 1;
            envc++;
        }

        // Map the kernel to 0xc0000000 (higher half)
        mmu_copy_kernel_pages(process_page_directory);

        mmu_free_pages(p->pdt, 0, (0xc0000000 >> 22) * 1024);
        bitmap_free_pages(p->pdt, 1);

        p->r.eip = (uintptr_t)exec_header->entry;
        p->r.ebp = (uintptr_t)0xc0000000 - 0x2;
        p->r.useresp = (uintptr_t)0xc0000000 - 0x10;
        p->pdt = process_page_directory;

        process_stack[2047] = 0xb02000;
        process_stack[2046] = 0xb00000;
        process_stack[2045] = argc;
    }
#if defined(__x86_64__)
    else if (header->version == ELF_ARCH_X86_64) {
        elf64_header *exec_header = (elf64_header *)addr;

        dbgprint("x86_64 ELF file\n");
        dbgprint("Entry point: %x\n", exec_header->entry);

        page_directory_table *process_page_directory = mmu_new_page_directory();
        uint64_t *process_stack = (uint64_t *)bitmap_alloc_contiguous_pages(2);

        elf64_program_header *program_header = (elf64_program_header *)(((uint8_t *)exec_header) + exec_header->program_header_offset);
        for (int i = 1; i <= exec_header->program_header_count; i++) {
            if (program_header->type == ELF_PT_LOAD) { // map to virtual address
                mmu_map_pages(process_page_directory, mmu_get_physical_address((uintptr_t)addr + program_header->offset), program_header->virtual_address, program_header->memory_size / BITMAP_PAGE_SIZE + 1, true, true, true);
            }

            program_header++;
        }

        // Map the ELF stack to 8 kiB below the kernel
        mmu_map_pages(process_page_directory, (uintptr_t)process_stack, (uintptr_t)0xc0000000 - 0x2000, 2, true, true, false);

        // Map 1024 pages for the heap
        void *heap = bitmap_alloc_contiguous_pages(1024);
        mmu_map_pages(process_page_directory, (uintptr_t)heap, (uintptr_t)0xc00000, 1024, true, true, false);

        void *process_argv = bitmap_alloc_page();
        mmu_map_pages(process_page_directory, (uintptr_t)process_argv, (uintptr_t)0xb00000, 1, true, true, false);
        memcpy(process_argv, argv, BITMAP_PAGE_SIZE);

        void *process_envp = bitmap_alloc_page();
        mmu_map_pages(process_page_directory, (uintptr_t)process_envp, (uintptr_t)0xb01000, 1, true, true, false);
        memcpy(process_envp, envp, BITMAP_PAGE_SIZE);

        // Map the kernel to 0xc0000000 (higher half)
        mmu_copy_kernel_pages(process_page_directory);

        mmu_free_pages(p->pdt, 0, (0xc0000000 >> 22) * 1024);
        bitmap_free_pages(p->pdt, 1);

        p->r.eip = (uintptr_t)exec_header->entry;
        p->r.ebp = (uintptr_t)0xc0000000 - 0x2;
        p->r.useresp = (uintptr_t)0xc0000000 - 0x20;
        p->pdt = process_page_directory;

        int argc = 0;
        while (argv[argc]) {
            argc++;
        }

        process_stack[1023] = 0xb01000;
        process_stack[1022] = 0xb00000;
        process_stack[1021] = argc;
    }
#endif
    else {
        dbgprint("Unsupported architecture: %x\n", header->version);
        return -ENOEXEC;
    }

    return 0;
}

int process_open_file(process_t *p, const char *path, int flags) {
    for (int i = 0; i < p->fd_count; i++) {
        file_descriptor *fd = &p->fd[i];
        if (!fd->used) {
            fd->used = true;
            fd->flags = flags;
            fd->offset = 0;
            strncpy(fd->name, path, 256);

            if ((flags & 0x3) == O_RDONLY) {
                fd->access.read = true;
            } else if ((flags & 0x3) == O_WRONLY) {
                fd->access.write = true;
            } else if ((flags & 0x3) == O_RDWR) {
                fd->access.read = true;
                fd->access.write = true;
            }

            if (strncmp(path, "/dev/tty", 8) == 0) {
                dbgprint("Opening TTY %d\n", atoi(path + 8));
                fd->type = S_IFCHR;
                int num = atoi(path + 8);

                if (num == 0) {
                    num = tty_get_current() + 1;
                } else if (num > tty_get_count()) {
                    return -ENOENT;
                }

                fd->tty = tty_open(num - 1);
            } else if (strncmp(path, "/dev/fb", 7) == 0) {
                dbgprint("Opening framebuffer %d\n", atoi(path + 7));
                fd->type = S_IFCHR;
                int num = atoi(path + 7);

                if (num > 0) {
                    return -ENOENT;
                }

                fd->fb = &fb0;
            } else {
                dbgprint("Opening file %s\n", path);
                fd->offset = 0;
                int stat_ret = 0;
                if ((stat_ret = vfs_stat(path, &p->cwd_mount, &p->cwd, &fd->mount, &fd->st))) {
                    fd->used = false;
                    return stat_ret;
                }

                fd->type = fd->st.st_mode & S_IFMT;

                if (flags & O_DIRECTORY) {
                    if (!(fd->st.st_mode & S_IFDIR)) {
                        fd->used = false;
                        return -ENOTDIR;
                    }
                }
            }

            return i;
        }
    }

    return -ENOMEM;
}

int process_stat_file(process_t *p, const char *path, struct stat *st) {
    return vfs_stat(path, &p->cwd_mount, &p->cwd, NULL, st);
}

int process_close_file(process_t *p, int fd) {
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

    process_t *p = process_by_pid(pid);
    if (!p) {
        return;
    }

    current_process = p;
}
