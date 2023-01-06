#include "kernel.h"

#define DEBUG 1

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ring3.h"
#include "bits.h"
#include "rootfs.h"
#include "cpu/acpi.h"
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
#include "drivers/ata.h"
#include "drivers/floppy.h"
#include "drivers/mbr.h"
#include "drivers/keyboard.h"
#include "drivers/pci.h"
#include "drivers/screen.h"
#include "drivers/serial.h"
#include "drivers/video/framebuffer.h"
#include "modules/timer.h"
#include "modules/multiboot2.h"
#include "modules/process.h"
#include "modules/kblayout/kb.h"
#include "modules/net/arp.h"
#include "modules/net/dns.h"
#include "modules/net/tcp.h"
#include "modules/net/udp.h"

static void iodriver_init(unsigned int boot_drive) {
    iodriver *_tmpio;
    if (ISSET_BIT_INT(boot_drive, 0x80)) {
        dbgprint("Booting from hard disk\n");
        //dbgwait();

        _tmpio = &ata_io;
        DISABLE_BIT_INT(boot_drive, 0x80);
    } else {
        dbgprint("Booting from floppy disk\n");
        //dbgwait();

        if (floppy_io.device == -2) {
            if (!floppy_init(NULL, 0, 0, 0)) {
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
    framebuffer_config fb = {
        .id = 2,
        .addr = 0xB8000,
        .pitch = 160,
        .width = 80,
        .height = 25,
        .bpp = 16,
        .type = FRAMEBUFFER_TYPE_TEXT,
    };
    size_t max_ram = 0;
    const char *boot_cmdline = NULL;
    void *acpi_rsdp_addr = NULL;
    unsigned int boot_drive = -1;
    unsigned int boot_partition = -1;
    screen_init(SCREEN_MODE_FRAMEBUFFER);

    framebuffer_setup(&fb);
    framebuffer_init();
    screen_clear();
    dbgprint("Kernel started\n");
    dbgprint("_esp: 0x%x\n", addr);

    floppy_io.device = -2;
    ata_io.device = -2;
    check_multiboot2(magic, addr);

    dbgprint("Multiboot2 magic number: %x\n", magic);

    for (struct multiboot_tag *tag = (struct multiboot_tag *)(addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *)((unsigned int)tag + ((tag->size + 7) & ~7))) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                boot_drive = ((struct multiboot_tag_bootdev *)tag)->biosdev;
                boot_partition = ((struct multiboot_tag_bootdev *)tag)->slice;
                break;

            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                max_ram = 1024 + ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper;
                break;

            case MULTIBOOT_TAG_TYPE_CMDLINE:
                boot_cmdline = ((struct multiboot_tag_string *)tag)->string;
                break;

            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
                struct multiboot_tag_framebuffer *fbtag = (struct multiboot_tag_framebuffer *)tag;
                fb.addr = fbtag->common.framebuffer_addr;
                fb.pitch = fbtag->common.framebuffer_pitch;
                fb.width = fbtag->common.framebuffer_width;
                fb.height = fbtag->common.framebuffer_height;
                fb.bpp = fbtag->common.framebuffer_bpp;
                fb.id = fbtag->common.framebuffer_type;
                switch (fb.id) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        fb.type = FRAMEBUFFER_TYPE_TEXT;
                        break;
                }

                dbgprint("Framebuffer: #%d %dx%dx%d %d @ 0x%x\n", fb.type, fb.width, fb.height, fb.bpp, fb.pitch, fb.addr);
                framebuffer_setup(&fb);

                dbgprint("Switching to Multiboot2 Framebuffer mode\n");
                //screen_init(SCREEN_MODE_FRAMEBUFFER);
                // screen_clear();

                break;
            }

            case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
                if (!acpi_rsdp_addr) {
                    struct multiboot_tag_old_acpi *acpi = (struct multiboot_tag_old_acpi *)tag;
                    acpi_rsdp_addr = (void *)acpi->rsdp;
                }
                break;
            }

            case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
                struct multiboot_tag_new_acpi *acpi = (struct multiboot_tag_new_acpi *)tag;
                acpi_rsdp_addr = (void *)acpi->rsdp;
                dbgprint("ACPI RSDP: 0x%x\n", acpi_rsdp_addr);
                break;
            }
        }
    }

    dbgprint("Boot device: %x:%x\n", boot_drive, boot_partition);
    dbgprint("Available RAM: %d MiB\n", max_ram / 1024);
    dbgprint("Boot command line: %s\n", boot_cmdline);

    if (boot_drive == -1 || boot_partition == -1) {
        panic("No boot device found from MBI.");
    }

    cpu_info cpuinfo;
    if (!get_cpuid_info(&cpuinfo)) {
        panic("CPUID not available");
    }
    dbgprint("CPUID Vendor ID: %.12s\n", cpuinfo.vendor_id);

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
    if (acpi_rsdp_addr) {
        acpi_init(acpi_rsdp_addr);
    } else {
        dbgprint("ACPI not available\n");
    }

    udp_init();
    dns_init();
    pci_init();
    dbgwait();
    dbgprint("Reading Master Boot Record...\n");
    iodriver_init(boot_drive);
    fs_init(boot_partition);
    dbgwait();
    dbgprint("Reading Root Directory...\n");
    rootfs.list_files(&rootfs_io, &rootfs);
    dbgwait();

    if (eth[0] && (eth[0]->ipv4.dns[0] || eth[0]->ipv4.dns[1] || eth[0]->ipv4.dns[2] || eth[0]->ipv4.dns[3])) {
        uint8_t ip[4];
        dns_query_ipv4(eth[0], eth[0]->ipv4.dns, "google.com", ip, 100);
    }

    dbgprint("Starting INIT\n");
    if (system("INIT.ELF")) {
        dbgwait();
        panic("INIT.ELF failed to load");
    }

    panic("INIT returned");
}
