#include "kernel.h"

#define DEBUG

// This variable stores the address to the target Ring 3
unsigned long int __ring3_addr;

registers *_panic_r;

void load_regs() {
    asm("mov %%esp, %0" : "=r"(_panic_r->esp));
    asm("mov %%ebp, %0" : "=r"(_panic_r->ebp));
    asm("mov %%edi, %0" : "=r"(_panic_r->edi));
    asm("mov %%esi, %0" : "=r"(_panic_r->esi));
    asm("mov %%ebx, %0" : "=r"(_panic_r->ebx));
    asm("mov %%edx, %0" : "=r"(_panic_r->edx));
    asm("mov %%ecx, %0" : "=r"(_panic_r->ecx));
    asm("mov %%eax, %0" : "=r"(_panic_r->eax));
    asm("mov %%ds, %0" : "=r"(_panic_r->ds));
    asm("mov %%es, %0" : "=r"(_panic_r->es));
    asm("mov %%fs, %0" : "=r"(_panic_r->fs));
    asm("mov %%gs, %0" : "=r"(_panic_r->gs));
    asm("mov %%ss, %0" : "=r"(_panic_r->ss));
    asm("mov %%cs, %0" : "=r"(_panic_r->cs));
    asm("pushf; pop %0" : "=r"(_panic_r->eflags));
    asm("mov $., %0" : "=r"(_panic_r->eip));
}

void panic_handler(const char *msg) {
    clear_screen();
    setcolor(RED << 4 | WHITE);
    printf("                                   MVLIRA05 OS                                  \n\n");
    setcolor(BLACK << 4 | WHITE);
    printf("PANIC!\n%s\n", msg);
    setcolor(BLACK << 4 | GRAY);
    // Print the registers before halting
    printf("eax: %08lx    ebx: %08lx    ecx: %08lx    edx: %08lx\n", _panic_r->eax, _panic_r->ebx, _panic_r->ecx, _panic_r->edx);
    printf("esi: %08lx    edi: %08lx    ebp: %08lx    esp: %08lx\n", _panic_r->esi, _panic_r->edi, _panic_r->ebp, _panic_r->esp);
    printf(" ds: %08lx     es: %08lx     fs: %08lx     gs: %08lx\n", _panic_r->ds, _panic_r->es, _panic_r->fs, _panic_r->gs);
    printf("eip: %08lx     cs: %08lx     ss: %08lx\n", _panic_r->eip, _panic_r->cs, _panic_r->ss);
    printf("eflags: %032lb\n", _panic_r->eflags);
    for (;;) {}
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

void kernel_main() {
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
    dbgprint("_esp: 0x%x\n", _esp);
    boot_drive = init_floppy();
    timer_init();
    init_keyboard();
    asm("sti");
    dbgprint("Interruptions enabled\n");
    dbgprint("Reading File Allocation Table...\n");
    loadfat(boot_drive);
    listfiles(boot_drive);

    dbgprint("Starting INIT\n");
    system("INIT.ELF");

    panic("Halting system\n");
}
