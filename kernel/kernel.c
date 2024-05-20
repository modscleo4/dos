#include "kernel.h"

#define DEBUG 1
#define DEBUG_SERIAL 0

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bits.h"
#include "cmdline.h"
#include "ring3.h"
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
#include "drivers/io/ata.h"
#include "drivers/io/floppy.h"
#include "drivers/mbr.h"
#include "drivers/keyboard.h"
#include "drivers/pci.h"
#include "drivers/serial.h"
#include "drivers/tty.h"
#include "drivers/video/framebuffer.h"
#include "modules/bitmap.h"
#include "modules/task.h"
#include "modules/timer.h"
#include "modules/vfs.h"
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
#include "modules/vfs/dev.h"
#include "modules/vfs/proc.h"
#include "modules/vfs/tmp.h"

uint32_t _esp;

static void iodriver_init(unsigned int boot_drive) {
    iodriver *_tmpio;
    if (boot_drive >= 0xE0 && boot_drive < 0xF0) {
        dbgprint("Booting from CD-ROM\n");

        _tmpio = &ata_io;
        boot_drive = ata_search_for_drive(boot_drive);
    } else if (boot_drive >= 0x80) {
        dbgprint("Booting from hard disk\n");

        _tmpio = &ata_io;
        boot_drive = ata_search_for_drive(boot_drive);
    } else {
        dbgprint("Booting from floppy disk\n");

        if (floppy_io.device == -2) {
            if (!floppy_init(NULL, 0, 0, 0)) {
                panic("Failed to initialize floppy driver");
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

static void fs_init(const char *boot_cmdline) {
    int drive = -1;
    int partition = -1;
    parse_cmdline_root(boot_cmdline, &drive, &partition);
    dbgprint("Root device: %x:%x\n", drive, partition);

    iodriver_init(drive);

    filesystem *_tmpfs = mbr_init(&rootfs_io, partition);
    if (!_tmpfs) {
        panic("No filesystem available");
    }

    rootfs = *_tmpfs;

    rootfs.init(&rootfs_io, &rootfs);
}

static void check_multiboot2(uint32_t magic, uint32_t addr) {
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

static void alloctest(void) {
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
}

static void memtest(const char *boot_cmdline) {
    if (!parse_cmdline_memtest(boot_cmdline)) {
        return;
    }

    dbgprint("Requesting all available memory...\n");
    size_t max_pages = bitmap_total_pages();
    uintptr_t *pages = calloc(max_pages, sizeof(uintptr_t));
    if (!pages) {
        panic("malloc failed");
    }

    for (size_t i = 0; i < max_pages; i++) {
        pages[i] = (uintptr_t)bitmap_alloc_page();
        if (!pages[i]) {
            break;
        }

        dbgprint_noinfo(".");
    }

    dbgprint_noinfo("\n");

    dbgprint("Freeing all allocated memory...\n");
    for (size_t i = 0; i < max_pages; i++) {
        if (!pages[i]) {
            continue;
        }

        bitmap_free_pages((void *)pages[i], 1);

        dbgprint_noinfo(".");
    }

    dbgprint_noinfo("\n");

    free(pages);
}

void kernel_int_wait(void) {
    // dbgprint("Waiting for interrupts...\n");
    interrupts_reenable();
    while (true) {
        process_t *init = process_by_pid(1);
        if (!init || init->state == PROCESS_ZOMBIE) {
            process_disable();
            panic("init process died");
        }

        task_t *task = task_pop();
        if (!task) {
            goto next;
        }

        task_execute(task);
        free(task);

    next:
        asm volatile("hlt");
    }
}

void kernel_main(uint32_t magic, uint32_t addr) {
    size_t max_ram = 0;
    char *boot_cmdline = "";
    void *acpi_rsdp_addr = NULL;
    int boot_drive = -1;
    int boot_partition = -1;

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
                fb0.addr   = (void *)fbtag->common.framebuffer_addr;
                fb0.pitch  = fbtag->common.framebuffer_pitch;
                fb0.width  = fbtag->common.framebuffer_width;
                fb0.height = fbtag->common.framebuffer_height;
                fb0.bpp    = fbtag->common.framebuffer_bpp;
                fb0.type   = fbtag->common.framebuffer_type;
                break;
            }

            case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
                if (!acpi_rsdp_addr) {
                    struct multiboot_tag_old_acpi *acpi = (struct multiboot_tag_old_acpi *)tag;
                    acpi_rsdp_addr = (void *)acpi->rsdp;
                    serial_write_str(SERIAL_COM1, "ACPI RSDP: 0x%x\n", acpi_rsdp_addr);
                }
                break;
            }

            case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
                struct multiboot_tag_new_acpi *acpi = (struct multiboot_tag_new_acpi *)tag;
                acpi_rsdp_addr = (void *)acpi->rsdp;
                serial_write_str(SERIAL_COM1, "ACPI RSDP: 0x%x\n", acpi_rsdp_addr);
                break;
            }
        }
    }

    mmu_init((uintptr_t)kernel_start_real_addr, (uintptr_t)kernel_end_real_addr, (uintptr_t)kernel_start_addr, (uintptr_t)kernel_end_addr);
    kernel_malloc_init();

    // ID Map the framebuffer
    mmu_map_pages(current_pdt, (uintptr_t)fb0.addr, (uintptr_t)fb0.addr, (fb0.width * fb0.height * fb0.bpp) / 8 / BITMAP_PAGE_SIZE + 1, true, false, true);

    serial_write_str(SERIAL_COM1, "Framebuffer 0: #%d %dx%dx%d %d @ 0x%x\n", fb0.type, fb0.width, fb0.height, fb0.bpp, fb0.pitch, fb0.addr);
    framebuffer_setup(&fb0);
    framebuffer_init(&fb0);
    tty_init(&fb0);

    dbgprint("Boot device: %x:%x\n", boot_drive, boot_partition);
    dbgprint("Available RAM: %d MiB\n", max_ram / 1024);
    dbgprint("Boot command line: %s\n", boot_cmdline);
    dbgprint("Kernel bounds at &%x &%x (&%x &%x)\n", kernel_start_addr, kernel_end_addr, kernel_start_real_addr, kernel_end_real_addr);

    if (boot_drive == -1) {
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

    alloctest();
    syscall_init();
    timer_init();
    keyboard_init();
    interrupts_enable();
    dbgprint("Interruptions enabled\n");
    if (acpi_rsdp_addr) {
        acpi_init((acpi_rsdp *)acpi_rsdp_addr);
    } else {
        dbgprint("ACPI not available\n");
    }

    memtest(boot_cmdline);
    dev_init();
    proc_init();
    tmp_init();
    process_init();
    udp_init();
    tcp_init();
    dns_init();
    pci_init();
    dbgprint("Reading Master Boot Record...\n");
    fs_init(boot_cmdline);
    vfs_init();
    vfs_mount(devfs_driver, devfs, "/dev");
    vfs_mount(procfs_driver, procfs, "/proc");
    vfs_mount(tmpfs_driver, tmpfs, "/tmp");
    vfs_describe_file(root_mount, &root_mount->rootdir, 0, true);

#if 0
    if (eth[0] && (eth[0]->ipv4.dns[0] || eth[0]->ipv4.dns[1] || eth[0]->ipv4.dns[2] || eth[0]->ipv4.dns[3])) {
        uint8_t ip[4];
        if (dns_query_ipv4(eth[0], eth[0]->ipv4.dns, "modscleo4.dev.br", ip, 1000)) {
            http_send_request(eth[0], ip, 80, "GET", "/", "modscleo4.dev.br");
        }
    }
#endif

    dbgprint("Starting INIT\n");
    if (system("/bin/init.elf")) {
        panic("init.elf failed to load");
    }
}
