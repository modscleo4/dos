#include "panic.h"

#include <stdio.h>
#include "../debug.h"
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
    clear_screen();
    setcolor(COLOR_RED << 4 | COLOR_WHITE);
    printf("                                   MVLIRA05 OS                                  \n\n");
    setcolor(COLOR_BLACK << 4 | COLOR_WHITE);
    printf("PANIC!\n%s\n", msg);
    setcolor(COLOR_BLACK << 4 | COLOR_GRAY);

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
        printf("eflags: %016b\n", r->eflags);
    }
    for (;;) {}
}
