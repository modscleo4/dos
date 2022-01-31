#include "panic.h"

#include <stdio.h>
#include "../debug.h"
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

void panic_handler(const char *msg, registers *r) {
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
            case 'i':
            case 'I':
                if (r) {
                    printf("\n");
                    // Print the registers before halting
                    printf("eax: %08lx    ebx: %08lx    ecx: %08lx    edx: %08lx\n", r->eax, r->ebx, r->ecx, r->edx);
                    printf("esi: %08lx    edi: %08lx    ebp: %08lx    esp: %08lx\n", r->esi, r->edi, r->ebp, r->esp);
                    printf("eip: %08lx    useresp: %08lx\n\n", r->eip, r->useresp);
                    hexdump(r->esp, 0x40);
                    printf("\n");
                    printf("cs: %04hx\n", (short int)r->cs);
                    printf("ds: %04hx\n", (short int)r->ds);
                    printf("es: %04hx\n", (short int)r->es);
                    printf("fs: %04hx\n", (short int)r->fs);
                    printf("gs: %04hx\n", (short int)r->gs);
                    printf("ss: %04hx\n", (short int)r->ss);

                    printf("eflags:");
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
                unsigned long int ebp = 0;
                if (r) {
                    ebp = r->ebp;
                } else {
                    asm volatile("mov %%ebp, %0" : "=r" (ebp));
                }
                callstack(ebp);
                break;
            }
        }

        gotoxy(0, 24);
        setcolor(COLOR_BLUE << 4 | COLOR_WHITE);
        printf("%-80s", r ? "<I> - Registers | <S> - Call Stack. | <Q> Restart." : "<S> - Call Stack. | <Q> Restart.");
        setcolor(COLOR_BLACK << 4 | COLOR_GRAY);

        c = getchar();
    } while (c != 'q' && c != 'Q');

    keyboard_clear_buffer();
    outb(KB_DATA_REGISTER, KB_RESET);

    while (true) {
        asm volatile("hlt");
    }
}
