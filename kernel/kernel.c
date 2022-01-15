#include "kernel.h"

static void iodriver_init(unsigned int edx) {
    iodriver *_tmpio;
    edx >>= 16;
    if (ISSET_BIT_INT(edx, 0x80)) {
        _tmpio = ata_init(DISABLE_BIT_INT(edx, 0x80));
    } else {
        _tmpio = floppy_init(edx);
    }

    if (!_tmpio) {
        panic("No I/O driver available");
    }

    io_driver = *_tmpio;

    if (io_driver.reset && io_driver.reset(io_driver.device)) {
        panic("Failed to reset device");
    }
}

static void fs_init(void) {
    filesystem *_tmpfs = mbr_init(&io_driver);
    if (!_tmpfs) {
        panic("No filesystem available");
    }

    fs = *_tmpfs;

    fs.init(&io_driver);
}

void kernel_main(unsigned int eax, unsigned int ebx, unsigned int ecx, unsigned int edx) {
    // Remove parameters from stack
    asm volatile("add $4, %%esp" : : : "memory");
    asm volatile("add $4, %%esp" : : : "memory");
    asm volatile("add $4, %%esp" : : : "memory");
    asm volatile("add $4, %%esp" : : : "memory");

    video_init(edx);
    clear_screen();
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

    timer_init();
    keyboard_init();
    asm("sti");
    dbgprint("Interruptions enabled\n");
    pci_init();
    dbgprint("Reading Master Boot Record...\n");
    iodriver_init(edx);
    fs_init();
    dbgprint("Reading Root Directory...\n");
    fs.list_files(&io_driver);

    dbgprint("Starting INIT\n");
    if (system("INIT.ELF")) {
        panic("INIT.ELF failed to load");
    }

    panic("INIT returned");
}
