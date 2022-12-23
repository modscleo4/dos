#include "kernel.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ring3.h"
#include "bits.h"
#include "rootfs.h"
#include "cpu/cpuid.h"
#include "cpu/fpu.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/irq.h"
#include "cpu/isr.h"
#include "cpu/mmu.h"
#include "cpu/panic.h"
#include "cpu/pic.h"
#include "cpu/syscall.h"
#include "debug.h"
#include "drivers/pci.h"
#include "drivers/ata.h"
#include "drivers/floppy.h"
#include "drivers/mbr.h"
#include "drivers/keyboard.h"
#include "drivers/screen.h"
#include "drivers/serial.h"
#include "modules/timer.h"
#include "modules/multiboot2.h"
#include "modules/process.h"
#include "modules/kblayout/kb.h"
#include "modules/net/arp.h"
#include "modules/net/udp.h"

static void iodriver_init(unsigned int boot_drive) {
    iodriver *_tmpio;
    if (ISSET_BIT_INT(boot_drive, 0x80)) {
        dbgprint("Booting from hard disk\n");
        //dbgwait();

        _tmpio = &ata_io;
        boot_drive = DISABLE_BIT_INT(boot_drive, 0x80);
    } else {
        dbgprint("Booting from floppy disk\n");
        //dbgwait();

        if (floppy_io.device == -2) {
            if (!floppy_init(NULL)) {
                dbgwait();
            }
        }
        _tmpio = &floppy_io;
    }

    if (!_tmpio || _tmpio->device == -2) {
        panic("No I/O driver available");
    }

    rootfs_io = *_tmpio;
    rootfs_io.device = boot_drive;

    if (rootfs_io.reset && rootfs_io.reset(&rootfs_io)) {
        panic("Failed to reset device");
    }
}

static void fs_init(unsigned int partition) {
    filesystem *_tmpfs = mbr_init(&rootfs_io, partition);
    if (!_tmpfs) {
        panic("No filesystem available");
    }

    rootfs = *_tmpfs;

    rootfs.init(&rootfs_io, &rootfs);
}

static void check_multiboot2(unsigned long int magic, unsigned long int addr) {
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        panic("Invalid magic number: %x", magic);
    }

    if (addr & 7) {
        panic("MBI not aligned: %x", addr);
    }
}

void kernel_main(unsigned long int magic, unsigned long int addr) {
    struct multiboot_tag *tag;
    unsigned int size;

    video_init(0);
    clear_screen();
    dbgprint("Kernel started\n");
    dbgprint("_esp: 0x%x\n", addr);

    floppy_io.device = -2;
    ata_io.device = -2;
    check_multiboot2(magic, addr);

    dbgprint("Multiboot2 magic number: %x\n", magic);

    unsigned int boot_drive = -1;
    unsigned int boot_partition = -1;
    for (tag = (struct multiboot_tag *)(addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *)((unsigned int)tag + ((tag->size + 7) & ~7))) {
        // dbgprint("Tag: %x\n", tag->type);
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                boot_drive = ((struct multiboot_tag_bootdev *)tag)->biosdev;
                boot_partition = ((struct multiboot_tag_bootdev *)tag)->slice;
                break;
        }
    }

    if (boot_drive == -1 || boot_partition == -1) {
        panic("No boot device found from MBI.");
    }

    cpu_info cpuinfo;
    if (!get_cpuid_info(&cpuinfo)) {
        panic("CPUID not available");
    }
    dbgprint("CPUID Vendor ID: %s\n", cpuinfo.vendor_id);

    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    mmu_init();
    syscall_init();
    pic_remap(32, 40);
    fpu_init(&cpuinfo);

    timer_init();
    serial_device *com1 = serial_init(SERIAL_COM1, 1);
    if (!com1) {
        dbgprint("Failed to initialize serial port COM1.\n");
    }
    serial_write_str(com1, "Serial port initialized.\r\n");
    keyboard_init();
    asm("sti");
    dbgprint("Interruptions enabled\n");
    dbgwait();
    udp_init();
    pci_init();
    dbgwait();
    dbgprint("Reading Master Boot Record...\n");
    iodriver_init(boot_drive);
    fs_init(boot_partition);
    dbgwait();
    dbgprint("Reading Root Directory...\n");
    rootfs.list_files(&rootfs_io, &rootfs);
    dbgwait();

    dbgprint("Starting INIT\n");
    if (system("INIT.ELF")) {
        dbgwait();
        panic("INIT.ELF failed to load");
    }

    panic("INIT returned");
}
