#include "kernel.h"

#define DEBUG 1

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ring3.h"
#include "bits.h"
#include "rootfs.h"
#include "cpu/acpi.h"
#include "cpu/cpuid.h"
#include "cpu/fpu.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/interrupts.h"
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
#include "modules/bitmap.h"
#include "modules/timer.h"
#include "modules/multiboot2.h"
#include "modules/process.h"
#include "modules/spinlock.h"
#include "modules/kblayout/kb.h"
#include "modules/net/arp.h"
#include "modules/net/dns.h"
#include "modules/net/http.h"
#include "modules/net/icmp.h"
#include "modules/net/tcp.h"
#include "modules/net/udp.h"

uint32_t _esp;

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

static int cmp(const void *a, const void *b) {
    return *(int *)b - *(int *)a;
}

void kernel_main(uint32_t magic, uint32_t addr) {
    framebuffer_config fb = {
        .addr = 0xB8000,
        .pitch = 160,
        .width = 80,
        .height = 25,
        .bpp = 16,
        .type = FRAMEBUFFER_TYPE_TEXT,
    };
    size_t max_ram = 0;
    char *boot_cmdline = "";
    void *acpi_rsdp_addr = NULL;
    unsigned int boot_drive = -1;
    unsigned int boot_partition = -1;

    serial_init(SERIAL_COM1, 1);
    serial_write_str(SERIAL_COM1, "Kernel started\n");
    serial_write_str(SERIAL_COM1, "_esp: 0x%x\n", _esp);

    floppy_io.device = -2;
    ata_io.device = -2;
    check_multiboot2(magic, addr);

    serial_write_str(SERIAL_COM1, "Multiboot2 magic number: %x\n", magic);
    serial_write_str(SERIAL_COM1, "Multiboot2 address: 0x%x\n", addr);

    for (struct multiboot_tag *tag = (struct multiboot_tag *)(addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *)((unsigned int)tag + ((tag->size + 7) & ~7))) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                boot_drive = ((struct multiboot_tag_bootdev *)tag)->biosdev;
                boot_partition = ((struct multiboot_tag_bootdev *)tag)->slice;
                break;

            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                max_ram = 1024 + ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper;
                bitmap_init(kernel_end_real_addr + 0x10000, max_ram * 1024);
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
                fb.type = fbtag->common.framebuffer_type;
                break;
            }

            case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
                if (!acpi_rsdp_addr) {
                    struct multiboot_tag_old_acpi *acpi = (struct multiboot_tag_old_acpi *)tag;
                    acpi_rsdp_addr = (void *)acpi->rsdp;
                    dbgprint("ACPI RSDP: 0x%x\n", acpi_rsdp_addr);
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

    mmu_init((uintptr_t)kernel_start_real_addr, (uintptr_t)kernel_end_real_addr, (uintptr_t)kernel_start_addr, (uintptr_t)kernel_end_addr);
    kernel_malloc_init();

    // ID Map the framebuffer
    mmu_map_pages(current_pdt, fb.addr, fb.addr, (fb.width * fb.height * fb.bpp) / 8 / BITMAP_PAGE_SIZE + 1, true, false, true);

    serial_write_str(SERIAL_COM1, "Framebuffer: #%d %dx%dx%d %d @ 0x%x\n", fb.type, fb.width, fb.height, fb.bpp, fb.pitch, fb.addr);
    screen_init(SCREEN_MODE_FRAMEBUFFER);
    framebuffer_setup(&fb);
    framebuffer_init();
    screen_clear();

    dbgprint("Boot device: %x:%x\n", boot_drive, boot_partition);
    dbgprint("Available RAM: %d MiB\n", max_ram / 1024);
    dbgprint("Boot command line: %s\n", boot_cmdline);
    dbgprint("Kernel bounds at &%x &%x (&%x &%x)\n", kernel_start_addr, kernel_end_addr, kernel_start_real_addr, kernel_end_real_addr);

    if (boot_drive == -1 || boot_partition == -1) {
        panic("No boot device found from MBI.");
    }

    if (max_ram == 0) {
        panic("No available RAM found from MBI.");
    } else if (max_ram < 31 * 1024) { // GRUB reports 1 MiB less than the actual amount of RAM
        panic("Not enough RAM. At least 32 MiB required.");
    }

    if (!get_cpuid_info()) {
        panic("CPUID not available");
    }
    dbgprint("CPUID Vendor ID: %.12s\n", cpuinfo.vendor_id);

    fpu_init();
    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    pic_remap(32, 40);
    dbgprint("GDT, IDT, ISR, IRQ and PIC initialized\n");

    dbgprint("Testing malloc...\n");
    int *arr = malloc(sizeof(int) * 10);
    if (!arr) {
        panic("malloc failed");
    }

    for (int i = 0; i < 10; i++) {
        arr[i] = i + 1;
    }

    char buf[100];
    memset(buf, 0, 100);
    for (int i = 0; i < 10; i++) {
        sprintf(buf + strlen(buf), "%d%c", arr[i], i == 9 ? '\n' : ' ');
    }
    dbgprint(buf);

    dbgprint("Testing qsort...\n");
    qsort(arr, 10, sizeof(int), cmp);
    memset(buf, 0, 100);
    for (int i = 0; i < 10; i++) {
        sprintf(buf + strlen(buf), "%d%c", arr[i], i == 9 ? '\n' : ' ');
    }
    dbgprint(buf);

    free(arr);

    syscall_init();
    timer_init();
    keyboard_init();
    interrupts_enable();
    dbgprint("Interruptions enabled\n");
    dbgwait();
    if (acpi_rsdp_addr) {
        acpi_init((acpi_rsdp *)acpi_rsdp_addr);
    } else {
        dbgprint("ACPI not available\n");
    }

    process_init();
    udp_init();
    tcp_init();
    dns_init();
    pci_init();
    //dbgwait();
    dbgprint("Reading Master Boot Record...\n");
    iodriver_init(boot_drive);
    fs_init(boot_partition);
    //dbgwait();
    dbgprint("Reading Root Directory...\n");
    rootfs.list_files(&rootfs_io, &rootfs);
    //dbgwait();

    if (eth[0] && (eth[0]->ipv4.dns[0] || eth[0]->ipv4.dns[1] || eth[0]->ipv4.dns[2] || eth[0]->ipv4.dns[3])) {
        uint8_t ip[4];
        if (dns_query_ipv4(eth[0], eth[0]->ipv4.dns, "modscleo4.dev.br", ip, 1000)) {
            http_send_request(eth[0], ip, 80, "GET", "/", "modscleo4.dev.br");
        }
    }

    dbgprint("Starting INIT\n");
    if (system("INIT.ELF")) {
        dbgwait();
        panic("INIT.ELF failed to load");
    }

    panic("INIT returned");
}
