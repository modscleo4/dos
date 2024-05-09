#include "panic.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include "acpi.h"
#include "gdt.h"
#include "interrupts.h"
#include "irq.h"
#include "power.h"
#include "../bits.h"
#include "../debug.h"
#include "../rootfs.h"
#include "../drivers/filesystem.h"
#include "../drivers/keyboard.h"
#include "../drivers/screen.h"
#include "../drivers/serial.h"
#include "../modules/timer.h"
#include "../modules/process.h"

static void read_gdt_segment(int (*write)(const char *format, ...), uint16_t segment) {
    gdt_ptr gdt_ptr;
    asm volatile("sgdt %0" : "=m"(gdt_ptr));

    gdt_entry ss = ((gdt_entry *)gdt_ptr.base)[segment / 8];
    write("( %04x|% 2x|% 3x) %08x %08x %d %d\n", segment / 8, ss.bits.code, ss.bits.DPL, ss.bits.base_high << 16 | ss.bits.base_low, ss.bits.limit_high << 16 | ss.bits.limit_low, ss.bits.granularity, ss.bits.code_data_segment);
}

static int serial_write_fn(const char *str, ...) {
    va_list args;
    va_start(args, str);
    serial_write_str_varargs(SERIAL_COM1, str, args);
    va_end(args);

    return 0;
}

static void register_dump(int (*write)(const char *format, ...), registers *r) {
    write("eax: %08lx    ebx: %08lx    ecx: %08lx    edx: %08lx\n", r->eax, r->ebx, r->ecx, r->edx);
    write("esi: %08lx    edi: %08lx    ebp: %08lx    esp: %08lx\n", r->esi, r->edi, r->ebp, r->esp);
    write("eip: %08lx    useresp: %08lx\n", r->eip, r->useresp);
    //write("cr0: %08lx    cr2: %08lx    cr3: %08lx    cr4: %08lx\n", r->cr0, r->cr2, r->cr3, r->cr4);
    write("%-79s\n", "");
    hexdump(write, (void *)r->esp, 0x40);
    write("SEG sltr(index|cd|dpl)     base    limit G D\n");
    write("cs: %04hx", r->cs); read_gdt_segment(write, r->cs);
    write("ds: %04hx", r->ds); read_gdt_segment(write, r->ds);
    write("es: %04hx", r->es); read_gdt_segment(write, r->es);
    write("fs: %04hx", r->fs); read_gdt_segment(write, r->fs);
    write("gs: %04hx", r->gs); read_gdt_segment(write, r->gs);
    write("ss: %04hx", r->ss); read_gdt_segment(write, r->ss);

    write("eflags: %x", r->eflags);
    if (r->eflags.carry)
        write(" CF");
    if (r->eflags.parity)
        write(" PF");
    if (r->eflags.adjust)
        write(" AF");
    if (r->eflags.zero)
        write(" ZF");
    if (r->eflags.sign)
        write(" SF");
    if (r->eflags.trap)
        write(" TF");
    if (r->eflags.interrupt)
        write(" IF");
    if (r->eflags.direction)
        write(" DF");
    if (r->eflags.overflow)
        write(" OF");
    if (r->eflags.iopl)
        write(" IOPL");
    if (r->eflags.nt)
        write(" NT");
    if (r->eflags.resume)
        write(" RF");
    if (r->eflags.virtual_86)
        write(" VM");
    if (r->eflags.alignment)
        write(" AC");
    if (r->eflags.virtual_interrupt)
        write(" VIF");
    if (r->eflags.virtual_interrupt_pending)
        write(" VIP");
    if (r->eflags.id)
        write(" ID");
    write(" IOPL: %d", r->eflags.iopl);
    write("\n");
}

static void panic_handler_varargs(registers *r, const char *file, const int line, const char *msg, va_list args) {
    serial_write_fn("\n=================================== PANIC ===================================\n");
    serial_write_str_varargs(SERIAL_COM1, msg, args);
    if (file) {
        serial_write_fn("\n at %s:%d\n", file, line);
    }
    if (r) {
        serial_write_fn("\n\n");
        register_dump(serial_write_fn, r);
    }
    serial_write_fn("=============================================================================\n");

    process *p = process_current();

    process_disable();

    for (int i = 0; i <= 15; i++) {
        if (i == IRQ_KEYBOARD || i == IRQ_ATA_PRIMARY || i == IRQ_ATA_SECONDARY || i == IRQ_FLOPPY || i == IRQ_PIT) {
            continue;
        }

        irq_uninstall_handler(i);
    }

    timer_disable_display();

    interrupts_reenable();

    char c = r ? 'R' : 'S';
    do {
        screen_clear();
        screen_setcolor(COLOR_RED << 4 | COLOR_WHITE);
        printf("                                  MVLIRA05 OS                                   \n");
        screen_setcolor(COLOR_BLACK << 4 | COLOR_WHITE);
        printf("PANIC!\n");
        vprintf(msg, args);
        if (file) {
            printf("\n at %s:%d\n", file, line);
        }
        printf("\n");
        screen_setcolor(COLOR_BLACK << 4 | COLOR_GRAY);

        switch (c) {
            default:
            case 'R':
                if (r) {
                    screen_gotoxy(0, 4);
                    printf("%-79s\n", "");
                    // Print the registers before halting
                    register_dump(printf, r);

                    break;
                }

            case 'S': {
                if (rootfs.type && rootfs_io.device >= 0) {
                    uint32_t ebp = 0;
                    if (r) {
                        ebp = r->ebp;
                    } else {
                        asm volatile("mov %%ebp, %0" : "=r"(ebp));
                    }

                    callstack(ebp, p);
                }
                break;
            }
        }

        if (interrupts_was_enabled()) {
            screen_gotoxy(0, -1);
            screen_setcolor(COLOR_BLUE << 4 | COLOR_WHITE);
            printf("%-80s", r ? "<R> Registers | <S> Call Stack | <Q> Restart | <P> Power Off." : "<S> Call Stack | <Q> Restart | <P> Power Off.");
            screen_setcolor(COLOR_BLACK << 4 | COLOR_GRAY);
        }

        do {
            c = toupper(getchar());
        } while (c != 'Q' && c != 'P' && c != 'R' && c != 'S'); // Ignore other keys
    } while (c != 'Q' && c != 'P');

    interrupts_disable();

    if (c == 'P') {
        power_shutdown();
    } else if (c == 'Q') {
        power_reboot();
    }

    while (true) {
        asm volatile("hlt");
    }
}

void _panic(const char *file, const int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    panic_handler_varargs(NULL, file, line, msg, args);
    va_end(args);
}

void panic_handler(registers *r, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    panic_handler_varargs(r, NULL, -1, msg, args);
    va_end(args);
}
