#include "debug.h"

#include <stdio.h>
#include <string.h>
#include "kernel.h"
#include "rootfs.h"
#include "drivers/keyboard.h"
#include "drivers/screen.h"
#include "modules/elf.h"

#ifndef DEBUG
#define DEBUG 1
#endif

void dbgprint(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
#if DEBUG
    vprintf(msg, args);
#endif
    va_end(args);
}

void dbgwait(void) {
#ifdef DEBUG
    getchar();
#endif
}

void hexdump(void *ptr, size_t n) {
    const uintptr_t ptr_i = (uintptr_t)ptr;
    unsigned char *ptr_c = ptr - ptr_i % 16;

    for (size_t i = 0; i < n; i++) {
        if (i % 16 == 0) {
            printf("%08x  ", ptr_i - ptr_i % 16 + i);
        }

        if (i < ptr_i % 16) {
            printf("   ");
        } else {
            printf("%02x ", ptr_c[i]);
            if (i % 16 == 15 || i == n - 1) {
                if (i % 16 < 15) {
                    for (int j = i % 16; j < 15; j++) {
                        printf("   ");
                    }
                }

                printf("  ");
                for (int j = i - (i % 16); j <= i; j++) {
                    if (j < ptr_i % 16) {
                        setcolor(COLOR_BLACK << 4 | COLOR_GRAY);
                        printf(" ");
                    } else {
                        if (ptr_c[j] >= 32 && ptr_c[j] <= 126) {
                            setcolor(COLOR_BLACK << 4 | COLOR_GREEN);
                            printf("%c", ptr_c[j]);
                        } else {
                            setcolor(COLOR_BLACK << 4 | COLOR_GRAY);
                            printf(".");
                        }
                    }

                    if (j == i - (i % 16) + 7) {
                        setcolor(COLOR_BLACK << 4 | COLOR_GRAY);
                        printf(" ");
                    }
                }

                setcolor(COLOR_BLACK << 4 | COLOR_GRAY);

                printf("\n");
            }
        }
    }

    printf("\n");
}

typedef struct stackframe {
    struct stackframe *ebp;
    uint32_t eip;
} stackframe;

void callstack(uint32_t ebp) {
    void *kernel_file_inode = rootfs.search_file(&rootfs_io, &rootfs, "KERNEL");
    if (!kernel_file_inode) {
        return;
    }

    if (!rootfs.load_file_at(&rootfs_io, &rootfs, kernel_file_inode, (void *)0x1000000)) {
        return;
    }

    elf32_header *kernel_header = (elf32_header *) 0x1000000;
    elf32_section_header *section_debuginfo = NULL;
    elf32_section_header *section_strtab = NULL;
    elf32_section_header *section_symtab = NULL;

    printf("\n");
    //dbgprint("  type: %hx\n", kernel_header->type);
    //dbgprint("  machine: %hx\n", kernel_header->machine);
    //dbgprint("  version: %x\n", kernel_header->version);
    //dbgprint("  entry: %x\n", kernel_header->entry);
    //dbgprint("  program_header_offset: %x\n", kernel_header->program_header_offset);
    //dbgprint("  section_header_offset: %x\n", kernel_header->section_header_offset);
    //dbgprint("  flags: %x\n", kernel_header->flags);
    //dbgprint("  header_size: %x\n", kernel_header->header_size);
    //dbgprint("  program_header_size: %x\n", kernel_header->program_header_size);
    //dbgprint("  program_header_count: %x\n", kernel_header->program_header_count);
    //dbgprint("  section_header_size: %x\n", kernel_header->section_header_size);
    //dbgprint("  section_header_count: %x\n", kernel_header->section_header_count);
    //dbgprint("  section_name_index: %x\n", kernel_header->section_name_index);

    if (kernel_header->section_header_count) {
        // Read section headers
        elf32_section_header *section_header = (elf32_section_header *) (((void *)kernel_header) + kernel_header->section_header_offset);

        elf32_section_header *section_name_header = (elf32_section_header *)(((void *)section_header) + (kernel_header->section_name_index) * kernel_header->section_header_size);

        for (int i = 1; i <= kernel_header->section_header_count; i++) {
            //dbgprint("  section %d:\n", i);
            unsigned char *section_name = (unsigned char *) ((void *)kernel_header) + section_name_header->offset + section_header->name;

            if (strcmp(section_name, ".debug_info") == 0) {
                section_debuginfo = section_header;
            } else if (strcmp(section_name, ".strtab") == 0) {
                section_strtab = section_header;
            } else if (strcmp(section_name, ".symtab") == 0) {
                section_symtab = section_header;
            }

            //dbgprint("    name: %s\n", section_name);
            //dbgprint("    type: %x\n", section_header->type);
            //dbgprint("    flags: %x\n", section_header->flags);
            //dbgprint("    address: %lx\n", section_header->address);
            //dbgprint("    offset: %lx\n", section_header->offset);
            //dbgprint("    size: %x\n", section_header->size);
            //dbgprint("    link: %x\n", section_header->link);
            //dbgprint("    info: %x\n", section_header->info);
            //dbgprint("    address_align: %x\n", section_header->address_align);
            //dbgprint("    entry_size: %x\n", section_header->entry_size);
            //dbgwait();
            section_header++;
        }
    }

    stackframe *stk;
    if (!ebp) {
        asm("movl %%ebp, %0" : "=r"(stk));
    } else {
        stk = (stackframe *) ebp;
    }

    printf("Call Stack:\n");
    while (stk) {
        printf("[%p]", stk->eip);
        if (stk->eip >= kernel_header->entry && stk->eip <= kernel_header->entry + rootfs.get_file_size(&rootfs, kernel_file_inode) && section_symtab) {
            uint32_t func_addr = 0;
            elf32_symbol_table_entry *func = NULL;

            elf32_symbol_table_entry *symbol_table = (elf32_symbol_table_entry *) ((void *)kernel_header + section_symtab->offset);
            for (int i = 1; i <= section_symtab->size; i++) {
                char *symbol_name = (char *)((void *)kernel_header) + section_strtab->offset + symbol_table->name;

                if (symbol_table->name) {
                    uint32_t symbol_address = symbol_table->value;
                    //if (ELF32_ST_TYPE(symbol_table->info) == ELF_SYMBOLTABLE_TYPE_FUNC) {
                        if (symbol_table->value > func_addr && symbol_table->value < stk->eip) {
                            func_addr = symbol_table->value;
                            func = symbol_table;
                        }
                    //}
                }

                //dbgprint("  symbol %d:\n", i);
                //dbgprint("    addr: %p\n", symbol_table);
                //dbgprint("    name: %p: %d\n", symbol_name, symbol_table->name);
                //dbgprint("    name: %s\n", symbol_name);
                //dbgprint("    value: %lx\n", symbol_table->value);
                //dbgprint("    size: %x\n", symbol_table->size);
                //dbgprint("    info: %x\n", symbol_table->info);
                //dbgprint("    other: %x\n", symbol_table->other);
                //dbgprint("    section: %x\n", symbol_table->section_index);
                //dbgwait();
                symbol_table++;
            }

            if (func) {
                printf(" %s (offset %x)", (unsigned char *)((void *)kernel_header) + section_strtab->offset + func->name, stk->eip - func->value);
            }
        }

        printf("\n");

        stk = stk->ebp;
    }
}
