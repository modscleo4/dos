#include "kernel.h"

#define DEBUG

// This variable stores the address to the target Ring 3
unsigned long int __ring3_addr;

void panic(const char *msg) {
    panic_handler(msg, NULL);
}

void panic_handler(const char *msg, registers *r) {
    clear_screen();
    setcolor(RED << 4 | WHITE);
    printf("                                   MVLIRA05 OS                                  \n\n");
    setcolor(BLACK << 4 | WHITE);
    printf("PANIC!\n%s\n", msg);
    setcolor(BLACK << 4 | GRAY);

    if (r) {
        printf("\n");
        // Print the registers before halting
        printf("eax: %08lx    ebx: %08lx    ecx: %08lx    edx: %08lx\n", r->eax, r->ebx, r->ecx, r->edx);
        printf("esi: %08lx    edi: %08lx    ebp: %08lx    esp: %08lx\n", r->esi, r->edi, r->ebp, r->esp);
        printf("eip: %08lx\n\n", r->eip);
        hexdump(r->esp, 0x40);
        printf("\n");
        printf("cs: %04hx\n", (short int) r->cs);
        printf("ds: %04hx\n", (short int) r->ds);
        printf("es: %04hx\n", (short int) r->es);
        printf("fs: %04hx\n", (short int) r->fs);
        printf("gs: %04hx\n", (short int) r->gs);
        printf("ss: %04hx\n", (short int) r->ss);
        printf("eflags: %016b\n", r->eflags);
    }
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
        printf("%02x ", ptr_c[i]);
        if (i % 16 == 15 || i == n - 1) {
            if (i % 16 < 15) {
                int j;
                for (j = i % 16; j < 15; j++) {
                    printf("   ");
                }
            }

            printf("\t");
            int j;
            for (j = i - (i % 16); j <= i; j++) {
                if (ptr_c[j] >= 32 && ptr_c[j] <= 126) {
                    printf("%c", ptr_c[j]);
                } else {
                    printf(".");
                }
            }

            printf("\n");
        }
    }

    printf("\n");
}

void kernel_main(long int eax, long int ebx, long int ecx, long int edx) {
    // Remove parameters from stack
    asm volatile("add $4, %%esp" : : : "memory");
    asm volatile("add $4, %%esp" : : : "memory");
    asm volatile("add $4, %%esp" : : : "memory");
    asm volatile("add $4, %%esp" : : : "memory");

    video_init(edx);
    //clear_screen();
    dbgprint("Kernel started\n");
    dbgprint("_esp: 0x%x\n", _esp);
    if (!get_cpuid_info(&cpuid)) {
        panic("CPUID not available");
    }
    dbgprint("CPUID Vendor ID: %s\n", cpuid.vendor_id);

    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    syscall_init();
    pic_remap(32, 40);
    fpu_init();

    boot_drive = floppy_init(edx);
    timer_init();
    keyboard_init();
    asm("sti");
    dbgprint("Interruptions enabled\n");
    dbgprint("Reading File Allocation Table...\n");
    fat_load(boot_drive);
    dbgprint("Reading Root Directory...\n");
    fat_listfiles(boot_drive);

    dbgprint("Starting INIT\n");
    if (system("INIT.ELF")) {
        panic("INIT.ELF failed to load");
    }

    panic("INIT returned");
}
