#include "kernel.h"

void kernel_main(void) {
    asm("push %edx");
    asm("push %edx");
    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    pic_remap(32, 40);

    asm("pop %edx");
    init_video();
    puts("Kernel started\n");
    asm("pop %edx");
    init_floppy();
    timer_init();
    init_keyboard();
    asm volatile ("sti");
    puts("Interruptions enabled\n");
    puts("Reading File Allocation Table...\n");
    loadfat();

    puts("Starting System Shell\n");
    system("shell");
}
