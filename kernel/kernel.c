#include "kernel.h"

#define DEBUG 1
#define DEBUG_SERIAL 0

#include <ctype.h>
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
#include "drivers/io/ata.h"
#include "drivers/io/floppy.h"
#include "drivers/mbr.h"
#include "drivers/keyboard.h"
#include "drivers/pci.h"
#include "drivers/screen.h"
#include "drivers/serial.h"
#include "drivers/video/framebuffer.h"
#include "modules/bitmap.h"
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

static void validate_cmdline_root(const char *cmdline) {
    // format: root=fdxpy, root=hdxpy, root=cdxpy
    if (strstr(cmdline, "root=")) { // check if root device is specified
        char *tmp = strstr(cmdline, "root=") + 5;
        if (strncmp(tmp, "fd", 2) == 0) { // floppy disk
            tmp += 2;
        } else if (strncmp(tmp, "hd", 2) == 0) {
            tmp += 2;
        } else if (strncmp(tmp, "cd", 2) == 0) {
            tmp += 2;
        } else {
            goto error;
        }

        char *start_disk = tmp;
        if (!isdigit(*start_disk)) { // check if the disk number start with a digit
            goto error;
        }

        char *end_disk = start_disk;
        while (*end_disk && isdigit(*end_disk)) {
            end_disk++;
        }

        if (*end_disk != 'p') { // check if there is a partition number
            goto error;
        }

        char *start_partition = end_disk + 1;
        if (!isdigit(*start_partition)) { // check if the partition number start with a digit
            goto error;
        }

        char *end_partition = start_partition;
        while (*end_partition && isdigit(*end_partition)) {
            end_partition++;
        }

        // check if there is anything after the partition number that is not a whitespace
        if (*end_partition != 0 && !isspace(*end_partition)) {
            goto error;
        }

        return;
    }

error:
    panic("Invalid root device specified");
}

static void parse_cmdline_root(const char *cmdline, int *drive, int *partition) {
    validate_cmdline_root(cmdline);

    char *tmp = strstr(cmdline, "root=");
    if (tmp) {
        tmp += 5;
        // format: root=fdxpy, root=hdxpy, root=cdxpy
        if (strncmp(tmp, "fd", 2) == 0) {
            *drive = 0;
            tmp += 2;
        } else if (strncmp(tmp, "hd", 2) == 0) {
            *drive = 0x80;
            tmp += 2;
        } else if (strncmp(tmp, "cd", 2) == 0) {
            *drive = 0xE0;
            tmp += 2;
        }

        char *end = tmp;
        while (*end && *end != 'p') {
            end++;
        }

        char tmp2 = tmp[end - tmp];
        tmp[end - tmp] = 0;
        *drive += strtol(tmp, NULL, 10);
        tmp[end - tmp] = tmp2;

        tmp = end + 1;

        end = tmp;
        while (*end && !isspace(*end)) {
            end++;
        }

        tmp2 = tmp[end - tmp];
        tmp[end - tmp] = 0;
        *partition = strtol(tmp, NULL, 10);
        tmp[end - tmp] = tmp2;
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

static void memtest(void) {
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
    }

    dbgprint("Freeing all allocated memory...\n");
    for (size_t i = 0; i < max_pages; i++) {
        if (!pages[i]) {
            continue;
        }

        bitmap_free_pages((void *)pages[i], 1);
    }

    free(pages);
}

void kernel_int_wait(void) {
    //dbgprint("Waiting for interrupts...\n");
    interrupts_reenable();
    while (true) {
        asm volatile("hlt");
    }
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

    //memtest();
    process_init();
    udp_init();
    tcp_init();
    dns_init();
    pci_init();
    dbgprint("Reading Master Boot Record...\n");
    fs_init(boot_cmdline);
    vfs_init();

    if (eth[0] && (eth[0]->ipv4.dns[0] || eth[0]->ipv4.dns[1] || eth[0]->ipv4.dns[2] || eth[0]->ipv4.dns[3])) {
        uint8_t ip[4];
        if (dns_query_ipv4(eth[0], eth[0]->ipv4.dns, "modscleo4.dev.br", ip, 1000)) {
            http_send_request(eth[0], ip, 80, "GET", "/", "modscleo4.dev.br");
        }
    }

    dbgprint("Starting INIT\n");
    if (system("/bin/init.elf")) {
        panic("init.elf failed to load");
    }
}
