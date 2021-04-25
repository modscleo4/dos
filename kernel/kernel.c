#include "kernel.h"

void panic(const char *msg) {
    printf("%s", msg);
    asm("hlt");
}

void hexdump(void *ptr, size_t n) {
    unsigned char *ptr_c = ptr;

    int i;
    for (i = 0; i < n; i++) {
        printf("%x ", ptr_c[i]);
    }

    printf("\n");
}

void start_shell(void) {
    system("SHELL   ");
}

int syscall(int sysno, ...) {
    va_list args;

    int arg0, arg1, arg2, arg3, arg4, arg5;

    va_start(args, sysno);
    arg0 = va_arg(args, int);
    arg1 = va_arg(args, int);
    arg2 = va_arg(args, int);
    arg3 = va_arg(args, int);
    arg4 = va_arg(args, int);
    arg5 = va_arg(args, int);
    va_end(args);

    int retval;

    asm("push %%ebp;"
        "mov %1, %%ebp;"
        "int $0x80;"
        "pop %%ebp"
        : "=g"(retval)
        : "g"(arg5), "a"(sysno), "b"(arg0), "c"(arg1), "d"(arg2), "S"(arg3), "D"(arg4)
        : "memory");

    return retval;
}

void kernel_main(unsigned int esp) {
    asm("push %edx");
    asm("push %edx");
    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    syscall_init();
    pic_remap(32, 40);

    asm("pop %edx");
    init_video();
    puts("Kernel started\n");
    printf("ESP: %x (%d)\n", esp, esp);
    asm("pop %edx");
    init_floppy();
    timer_init();
    init_keyboard();
    asm volatile("sti");
    puts("Interruptions enabled\n");
    puts("Testing INT 0x80(1)\n");
    printf("Syscall return: %d\n", syscall(1, 2, 3, 4, 5, 6, 7));
    puts("Testing INT 0x80(0)\n");
    printf("Syscall return: %d\n", syscall(0, 2, 3, 4, 5, 6, 7));
    puts("Reading File Allocation Table...\n");
    loadfat();
    listfiles();

    printf("Starting SHELL\n");
    start_shell();

    printf("Halting system\n");
    for (;;) {}
}
