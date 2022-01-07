#include "kernel.h"

#define DEBUG

// This variable stores the address to the target Ring 3
unsigned long int __ring3_addr;

void panic(const char *msg) {
    printf("%s", msg);
    asm("hlt");
}

void dbgprint(const char *msg, ...) {
    #ifdef DEBUG
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
    #endif
}

void hexdump(void *ptr, size_t n) {
    unsigned char *ptr_c = ptr;

    int i;
    for (i = 0; i < n; i++) {
        printf("%x ", ptr_c[i]);
        if (i % 16 == 15) {
            printf("\n");
        }
    }

    printf("\n");
}

void start_shell(void) {
    system("SHELL");
}

void kernel_main(uintptr_t esp) {
    // Load _esp from %esp register
    //esp -= 0x20;
    //_esp = esp;

    asm("push %edx");
    asm("push %edx");
    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    syscall_init();
    pic_remap(32, 40);
    fpu_init();

    asm("cli");
    asm("pop %edx");
    init_video();
    //clear_screen();
    dbgprint("Kernel started\n");
    asm("pop %edx");
    dbgprint("_esp: %x\n", _esp);
    init_floppy();
    timer_init();
    init_keyboard();
    asm("sti");
    dbgprint("Interruptions enabled\n");
    dbgprint("Reading File Allocation Table...\n");
    loadfat();
    listfiles();

    dbgprint("Starting SHELL\n");
    start_shell();

    dbgprint("Halting system\n");
    for (;;) {}
}
