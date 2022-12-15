#include "panic.h"

#include <stdio.h>
#include "gdt.h"
#include "irq.h"
#include "../bits.h"
#include "../debug.h"
#include "../drivers/filesystem.h"
#include "../drivers/keyboard.h"
#include "../drivers/screen.h"

void panic(const char *msg, ...) {
    char buf[1024];

    va_list args;
    va_start(args, msg);
    vsprintf(buf, msg, args);
    va_end(args);

    panic_handler(buf, NULL);
}

static void read_gdt_segment(unsigned short int segment) {
    gdt_ptr gdt_ptr;
    asm volatile("sgdt %0" : "=m"(gdt_ptr));

    gdt_entry ss = ((gdt_entry *)gdt_ptr.base)[segment / 8];
    printf("( %04x|% 2x|% 3x) %08x %08x %d %d\n", segment / 8, ss.bits.code, ss.bits.DPL, ss.bits.base_high << 16 | ss.bits.base_low, ss.bits.limit_high << 16 | ss.bits.limit_low, ss.bits.granularity, ss.bits.code_data_segment);
}

void panic_handler(const char *msg, registers *r) {
    irq_uninstall_handler(IRQ_PIT);
    irq_uninstall_handler(IRQ_FLOPPY);
    irq_uninstall_handler(IRQ_CMOS);
    irq_uninstall_handler(IRQ_ATA_PRIMARY);
    irq_uninstall_handler(IRQ_ATA_SECONDARY);
    asm volatile("sti");

    char c = r ? 'I' : 'S';
    do {
        clear_screen();
        setcolor(COLOR_RED << 4 | COLOR_WHITE);
        printf("                                   MVLIRA05 OS                                  \n\n");
        setcolor(COLOR_BLACK << 4 | COLOR_WHITE);
        printf("PANIC!\n%s\n", msg);
        setcolor(COLOR_BLACK << 4 | COLOR_GRAY);

        switch (c) {
            default:
            case 'i':
            case 'I':
                if (r) {
                    printf("\n");
                    // Print the registers before halting
                    printf("eax: %08lx    ebx: %08lx    ecx: %08lx    edx: %08lx\n", r->eax, r->ebx, r->ecx, r->edx);
                    printf("esi: %08lx    edi: %08lx    ebp: %08lx    esp: %08lx\n", r->esi, r->edi, r->ebp, r->esp);
                    printf("eip: %08lx    useresp: %08lx\n\n", r->eip, r->useresp);
                    hexdump(r->esp, 0x40);
                    printf("SEG sltr(index|cd|dpl)     base    limit G D\n");
                    printf("cs: %04hx", (short int)r->cs);
                    read_gdt_segment(r->cs);
                    printf("ds: %04hx", (short int)r->ds);
                    read_gdt_segment(r->ds);
                    printf("es: %04hx", (short int)r->es);
                    read_gdt_segment(r->es);
                    printf("fs: %04hx", (short int)r->fs);
                    read_gdt_segment(r->fs);
                    printf("gs: %04hx", (short int)r->gs);
                    read_gdt_segment(r->gs);
                    printf("ss: %04hx\n", (short int)r->ss);

                    printf("eflags: %x", r->eflags);
                    if (r->eflags.carry) printf(" CF");
                    if (r->eflags.parity) printf(" PF");
                    if (r->eflags.adjust) printf(" AF");
                    if (r->eflags.zero) printf(" ZF");
                    if (r->eflags.sign) printf(" SF");
                    if (r->eflags.trap) printf(" TF");
                    if (r->eflags.interrupt) printf(" IF");
                    if (r->eflags.direction) printf(" DF");
                    if (r->eflags.overflow) printf(" OF");
                    if (r->eflags.iopl) printf(" IOPL");
                    if (r->eflags.nt) printf(" NT");
                    if (r->eflags.resume) printf(" RF");
                    if (r->eflags.virtual_86) printf(" VM");
                    if (r->eflags.alignment) printf(" AC");
                    if (r->eflags.virtual_interrupt) printf(" VIF");
                    if (r->eflags.virtual_interrupt_pending) printf(" VIP");
                    if (r->eflags.id) printf(" ID");
                    printf(" IOPL: %d", r->eflags.iopl);
                    printf("\n");

                    break;
                }

            case 's':
            case 'S': {
                if (rootfs.type) {
                    unsigned long int ebp = 0;
                    if (r) {
                        ebp = r->ebp;
                    } else {
                        asm volatile("mov %%ebp, %0" : "=r" (ebp));
                    }
                    callstack(ebp);
                }
                break;
            }
        }

        gotoxy(0, 24);
        setcolor(COLOR_BLUE << 4 | COLOR_WHITE);
        printf("%-80s", r ? "<I> - Registers | <S> - Call Stack. | <Q> Restart." : "<S> - Call Stack. | <Q> Restart.");
        setcolor(COLOR_BLACK << 4 | COLOR_GRAY);

        c = getchar();
    } while (c != 'q' && c != 'Q');

    asm volatile ("cli");

    keyboard_clear_buffer();
    outb(KB_DATA_REGISTER, KB_RESET);

    while (true) {
        asm volatile("hlt");
    }
}
