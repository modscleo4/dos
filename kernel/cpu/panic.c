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

static void read_gdt_segment(uint16_t segment) {
    gdt_ptr gdt_ptr;
    asm volatile("sgdt %0" : "=m"(gdt_ptr));

    gdt_entry ss = ((gdt_entry *)gdt_ptr.base)[segment / 8];
    printf("( %04x|% 2x|% 3x) %08x %08x %d %d\n", segment / 8, ss.bits.code, ss.bits.DPL, ss.bits.base_high << 16 | ss.bits.base_low, ss.bits.limit_high << 16 | ss.bits.limit_low, ss.bits.granularity, ss.bits.code_data_segment);
}

static void panic_handler_varargs(registers *r, const char *msg, va_list args) {
    serial_write_str(SERIAL_COM1, "==================== PANIC ====================\n");
    serial_write_str(SERIAL_COM1, "%s\n", msg);
    serial_write_str(SERIAL_COM1, "================================================\n");

    for (int i = 0; i <= 15; i++) {
        if (i == IRQ_KEYBOARD || i == IRQ_ATA_PRIMARY || i == IRQ_ATA_SECONDARY || i == IRQ_FLOPPY) {
            continue;
        }

        irq_uninstall_handler(i);
    }

    interrupts_reenable();


    char c = r ? 'R' : 'S';
    do {
        screen_clear();
        screen_setcolor(COLOR_RED << 4 | COLOR_WHITE);
        printf("                                   MVLIRA05 OS                                  \n");
        screen_setcolor(COLOR_BLACK << 4 | COLOR_WHITE);
        printf("PANIC!\n");
        vprintf(msg, args);
        printf("\n");
        screen_setcolor(COLOR_BLACK << 4 | COLOR_GRAY);

        switch (c) {
            default:
            case 'R':
                if (r) {
                    printf("\n");
                    // Print the registers before halting
                    printf("eax: %08lx    ebx: %08lx    ecx: %08lx    edx: %08lx\n", r->eax, r->ebx, r->ecx, r->edx);
                    printf("esi: %08lx    edi: %08lx    ebp: %08lx    esp: %08lx\n", r->esi, r->edi, r->ebp, r->esp);
                    printf("eip: %08lx    useresp: %08lx\n\n", r->eip, r->useresp);
                    hexdump((void *)r->esp, 0x40);
                    printf("SEG sltr(index|cd|dpl)     base    limit G D\n");
                    printf("cs: %04hx", r->cs);
                    read_gdt_segment(r->cs);
                    printf("ds: %04hx", r->ds);
                    read_gdt_segment(r->ds);
                    printf("es: %04hx", r->es);
                    read_gdt_segment(r->es);
                    printf("fs: %04hx", r->fs);
                    read_gdt_segment(r->fs);
                    printf("gs: %04hx", r->gs);
                    read_gdt_segment(r->gs);
                    printf("ss: %04hx\n", r->ss);

                    printf("eflags: %x", r->eflags);
                    if (r->eflags.carry)
                        printf(" CF");
                    if (r->eflags.parity)
                        printf(" PF");
                    if (r->eflags.adjust)
                        printf(" AF");
                    if (r->eflags.zero)
                        printf(" ZF");
                    if (r->eflags.sign)
                        printf(" SF");
                    if (r->eflags.trap)
                        printf(" TF");
                    if (r->eflags.interrupt)
                        printf(" IF");
                    if (r->eflags.direction)
                        printf(" DF");
                    if (r->eflags.overflow)
                        printf(" OF");
                    if (r->eflags.iopl)
                        printf(" IOPL");
                    if (r->eflags.nt)
                        printf(" NT");
                    if (r->eflags.resume)
                        printf(" RF");
                    if (r->eflags.virtual_86)
                        printf(" VM");
                    if (r->eflags.alignment)
                        printf(" AC");
                    if (r->eflags.virtual_interrupt)
                        printf(" VIF");
                    if (r->eflags.virtual_interrupt_pending)
                        printf(" VIP");
                    if (r->eflags.id)
                        printf(" ID");
                    printf(" IOPL: %d", r->eflags.iopl);
                    printf("\n");

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
                    callstack(ebp);
                }
                break;
            }
        }

        if (interrupts_was_enabled()) {
            screen_gotoxy(0, -1);
            screen_setcolor(COLOR_BLUE << 4 | COLOR_WHITE);
            printf("%-80s", r ? "<R> - Registers | <S> - Call Stack | <Q> Restart | <P> - Power Off." : "<S> - Call Stack | <Q> Restart | <P> - Power Off.");
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

void panic(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    panic_handler_varargs(NULL, msg, args);
    va_end(args);
}

void panic_handler(registers *r, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    panic_handler_varargs(r, msg, args);
    va_end(args);
}
