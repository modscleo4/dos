#include "floppy.h"
#include "../kernel.h"
#include <stdio.h>
#include <string.h>

unsigned int boot_drive = 0;
unsigned int drives[2] = {0, 0};
bios_params params;
floppy_parameters floppy;
int floppy_motor_state[2] = {0, 0}; /* 0: OFF, 1: ON; 2: WAIT */

int irq6_c = 0;

void buffer2struct(unsigned char *buffer, bios_params *p) {
    memcpy(p, &buffer[11], sizeof(bios_params));
}

void init_floppy() {
    unsigned int edx = 0;
    asm("mov %%edx, %0;"
        : "=r"(edx));
    boot_drive = edx >> 16U;;

    memcpy(&floppy, (unsigned char *)DISK_PARAMETER_ADDRESS, sizeof(floppy_parameters));
    irq_install_handler(6, floppy_handler);

    detect_floppy_types();
    dbgprint("Boot drive is #%d\n", boot_drive);
    dbgprint("  Head settle time: %d\n", floppy.head_settle_time);
}

void detect_floppy_types() {
    unsigned char c = read_cmos_register(0x10, 1);

    drives[0] = c >> 4U;
    drives[1] = c & 0xFU;

    dbgprint("Floppy 0: %s\n", drive_types[drives[0]]);
    dbgprint("Floppy 1: %s\n", drive_types[drives[1]]);
}

void loadfat() {
    ResetFloppy(boot_drive);
    unsigned char buffer[512];
    floppy_sector_read(boot_drive, 0, buffer);

    buffer2struct(buffer, &params);
    char volume_label[12];
    char filesystem[9];
    strncpy(volume_label, params.volume_label, 11);
    strncpy(filesystem, params.filesystem, 8);

    printf("Volume label is %s\n", volume_label);
    printf("File system is %s\n", filesystem);
    printf("Serial number is %X\n", params.serial_number);
}

void lba2chs(unsigned long int lba, chs *c, floppy_parameters fparams) {
    c->cylinder = lba / (2 * fparams.sectors_per_track);
    c->head = ((lba % (2 * fparams.sectors_per_track)) / fparams.sectors_per_track);
    c->sector = ((lba % (2 * fparams.sectors_per_track)) % fparams.sectors_per_track + 1);
}

void wait_irq6() {
    while (irq6_c <= 0) {}
    irq6_c--;
}

int floppy_recv_byte(unsigned int drive) {
    int base = (drive == 0) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;

    int i;
    for (i = 0; i < 600; i++) {
        timer_wait(1);
        if (0x80U & inb(base + MAIN_STATUS_REGISTER)) {
            return inb(base + DATA_FIFO);
        }
    }

    return -1;
}

int floppy_send_byte(unsigned int drive, unsigned char b) {
    int base = (drive == 0) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;

    int i;
    for (i = 0; i < 600; i++) {
        timer_wait(1);
        if (0x80U & inb(base + MAIN_STATUS_REGISTER)) {
            outb(base + DATA_FIFO, b);
            return 0;
        }
    }

    return -1;
}

void floppy_check_interrupt(unsigned int drive, int *st0, int *cylinder) {
    floppy_send_byte(drive, SENSE_INTERRUPT);
    *st0 = floppy_recv_byte(drive);
    *cylinder = floppy_recv_byte(drive);
}

int floppy_calibrate(unsigned int drive) {
    const char *fn = "floppy_calibrate";
    int i;
    int st0 = 0;
    int cylinder = -1;

    floppy_motor_on(drive);

    for (i = 0; i < 10; i++) {
        floppy_send_byte(drive, RECALIBRATE);
        floppy_send_byte(drive, drive);

        wait_irq6();
        floppy_check_interrupt(drive, &st0, &cylinder);

        if (0xC0 & st0) {
            static const char *status[] = {0, "error", "invalid", "drive"};
            dbgprint("%s: status = %s\n", fn, status[st0 >> 6]);
            continue;
        }

        if (!cylinder) {
            floppy_motor_off(drive);
            return 0;
        }
    }

    dbgprint("%s: 10 retries exhausted\n", fn);
    floppy_motor_off(drive);

    return -1;
}

int ResetFloppy(unsigned int drive) {
    int base = (drive == 0) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;
    outb(base + DIGITAL_OUTPUT_REGISTER, 0x00);
    outb(base + DIGITAL_OUTPUT_REGISTER, 0x0C);

    wait_irq6();

    int i;
    for (i = 0; i < 4; i++) {
        int st0 = 0;
        int cylinder = -1;
        floppy_check_interrupt(drive, &st0, &cylinder);
    }

    outb(base + CONFIGURATION_CONTROL_REGISTER, 0x00);
    floppy_send_byte(drive, SPECIFY);
    floppy_send_byte(drive, 0xDF);
    floppy_send_byte(drive, 0x02);

    if (floppy_calibrate(drive)) {
        return -1;
    }

    return 0;
}

int floppy_seek(unsigned int drive, unsigned char cylinder, unsigned char head) {
    const char *fn = "floppy_seek";
    int st0 = 0;
    int cyl = -1;

    floppy_motor_on(drive);

    int i;
    for (i = 0; i < 10; i++) {
        floppy_send_byte(drive, SEEK);
        floppy_send_byte(drive, head << 2);
        floppy_send_byte(drive, cylinder);

        wait_irq6();
        floppy_check_interrupt(drive, &st0, &cyl);

        if (0xC0 & st0) {
            static const char *status[] = {"normal", "error", "invalid", "drive"};
            dbgprint("%s: status = %s\n", fn, status[st0 >> 6]);
            continue;
        }

        if (cyl == cylinder) {
            floppy_motor_off(drive);
            return 0;
        }
    }

    dbgprint("%s: 10 retries exhausted\n", fn);
    floppy_motor_off(drive);

    return -1;
}

void floppy_motor_on(unsigned int drive) {
    int base = (drive == 0) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;
    if (floppy_motor_state[drive] != 1) {
        outb(base + DIGITAL_OUTPUT_REGISTER, 0x1C);
        timer_wait(50);
    }

    floppy_motor_state[drive] = 1;
}

void floppy_motor_off(unsigned int drive) {
    int base = (drive == 0) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;
    if (floppy_motor_state[drive] != 0) {
        timer_wait(500);
        outb(base + DIGITAL_OUTPUT_REGISTER, 0x0C);
        floppy_motor_state[drive] = 0;
    }
}

void floppy_handler(registers *r) {
    irq6_c++;
}

static void floppy_dma_init(floppy_direction direction, unsigned char *buffer) {
    const char *fn = "floppy_dma_init";
    union {
        unsigned char b[4];
        unsigned long int l;
    } addr, count;

    addr.l = (unsigned long int)buffer;
    count.l = (unsigned long int)511;

    if ((addr.l >> 24) || (count.l >> 16) || (((addr.l & 0xffff) + count.l) >> 16)) {
        dbgprint("%s: buffer problem\n", fn);
        asm("hlt");
    }

    unsigned char mode;

    switch (direction) {
        case floppy_direction_read:
            mode = 0x46;
            break;

        case floppy_direction_write:
            mode = 0x4a;
            break;

        default:
            dbgprint("%s: invalid direction", fn);
            asm("hlt");
            return;
    }

    outb(0x0a, 0x06);
    outb(0x0c, 0xff);
    outb(0x04, addr.b[0]);
    outb(0x04, addr.b[1]);
    outb(0x81, addr.b[2]);
    outb(0x0c, 0xff);
    outb(0x05, count.b[0]);
    outb(0x05, count.b[1]);
    outb(0x0b, mode);
    outb(0x0a, 0x02);
}

int floppy_do_sector(unsigned int drive, unsigned long int lba, unsigned char *buffer, floppy_direction direction) {
    const char *fn = "floppy_do_sector";
    unsigned char command;
    static const int flags = 0xC0;

    chs c;
    lba2chs(lba, &c, floppy);

    switch (direction) {
        case floppy_direction_read:
            command = READ_DATA | flags;
            break;

        case floppy_direction_write:
            command = WRITE_DATA | flags;
            break;

        default:
            dbgprint("%s: invalid direction", fn);
            asm("hlt");
            return 0;
    }

    if (floppy_seek(drive, c.cylinder, c.head)) {
        return -1;
    }

    int i;
    for (i = 0; i < 20; i++) {
        floppy_motor_on(drive);
        floppy_dma_init(direction, buffer);
        timer_wait(floppy.head_settle_time);

        floppy_send_byte(drive, command);
        floppy_send_byte(drive, (c.head << 2) | drive);
        floppy_send_byte(drive, c.cylinder);
        floppy_send_byte(drive, c.head);
        floppy_send_byte(drive, c.sector);
        floppy_send_byte(drive, floppy.bytes_per_sector);
        floppy_send_byte(drive, floppy.sectors_per_track);
        floppy_send_byte(drive, floppy.gap_length);
        floppy_send_byte(drive, floppy.data_length);

        wait_irq6();

        unsigned char st0, st1, st2, rcy, rhe, rse, bps;
        st0 = floppy_recv_byte(drive);
        st1 = floppy_recv_byte(drive);
        st2 = floppy_recv_byte(drive);
        rcy = floppy_recv_byte(drive);
        rhe = floppy_recv_byte(drive);
        rse = floppy_recv_byte(drive);
        bps = floppy_recv_byte(drive);

        int error = 0;

        if (st0 & 0xC0) {
            static const char *status[] = {0, "error", "invalid command", "drive not ready"};
            dbgprint("%s: status = %s\n", fn, status[st0 >> 6]);
            error = 1;
        }

        if (st1 & 0x80) {
            dbgprint("%s: end of cylinder\n", fn);
            error = 1;
        }

        if (st0 & 0x08) {
            dbgprint("%s: drive not ready\n", fn);
            error = 1;
        }

        if (st1 & 0x20) {
            dbgprint("%s: CRC error\n", fn);
            error = 1;
        }

        if (st1 & 0x10) {
            dbgprint("%s: controller timeout\n", fn);
            error = 1;
        }

        if (st1 & 0x04) {
            dbgprint("%s: no data found\n", fn);
            error = 1;
        }

        if ((st1 | st2) & 0x01) {
            dbgprint("%s: no address mark found\n", fn);
            error = 1;
        }

        if (st2 & 0x40) {
            dbgprint("%s: deleted address mark\n", fn);
            error = 1;
        }

        if (st2 & 0x20) {
            dbgprint("%s: CRC error in data\n", fn);
            error = 1;
        }

        if (st2 & 0x10) {
            dbgprint("%s: wrong cylinder\n", fn);
            error = 1;
        }

        if (st2 & 0x04) {
            dbgprint("%s: uPD765 sector not found\n", fn);
            error = 1;
        }

        if (st2 & 0x02) {
            dbgprint("%s: bad cylinder\n", fn);
            error = 1;
        }

        if (bps != 0x2) {
            dbgprint("%s: wanted 512B/sector, got %d", fn, (1 << (bps + 7)));
            error = 1;
        }

        if (st1 & 0x02) {
            dbgprint("%s: not writable\n", fn);
            error = 2;
        }

        if (!error) {
            floppy_motor_off(drive);

            return 0;
        }

        if (error > 1) {
            dbgprint("%s: not retrying..\n", fn);
            floppy_motor_off(drive);

            return -2;
        }
    }

    dbgprint("%s: 20 retries exhausted\n", fn);
    floppy_motor_off(drive);

    return -1;
}

int floppy_sector_read(unsigned int drive, unsigned long int lba, unsigned char *data) {
    return floppy_do_sector(drive, lba, data, floppy_direction_read);
}

int floppy_sector_write(unsigned int drive, unsigned long int lba, unsigned char *data) {
    return floppy_do_sector(drive, lba, data, floppy_direction_write);
}

int floppy_search_file(const char *filename, fat_entry *f) {
    unsigned char buffer[512];

    int rootdir_sector = params.reserved_sectors + params.number_of_fat * params.sectors_per_fat;
    char _fname[9];
    strncpy(_fname, filename, 8);
    int _len = strlen(_fname);
    while (_len < 8) {
        _fname[_len++] = ' ';
    }

    int i;
    for (i = 0; i < params.rootdir_entries * sizeof(fat_entry); i += sizeof(fat_entry)) {
        if (i % 512 == 0) {
            floppy_sector_read(boot_drive, rootdir_sector++, buffer);
        }

        if (buffer[i % 512] == 0) {
            continue;
        }

        buffer2fatentry(&buffer[i % 512], f);

        char fname[9];
        strncpy(fname, f->name, 8);

        if (strcmp(fname, _fname) == 0) {
            return 0;
        }
    }

    return -1;
}

void *floppy_load_file_at(const fat_entry *f, void *addr) {
    if (!f) {
        return NULL;
    }

    unsigned short int cluster = f->cluster;
    unsigned int first_fat_sector = params.reserved_sectors;

    unsigned int cl = 0;
    unsigned int last_fat_sector = 0;

    unsigned char fat_buffer[512];

    unsigned int rootdir_sector = params.reserved_sectors + params.number_of_fat * params.sectors_per_fat;

    while (cluster < 0xFF8) {
        unsigned int fat_offset = cluster + (cluster / 2);
        unsigned int fat_sector = first_fat_sector + (fat_offset / params.bytes_per_sector);
        unsigned int ent_offset = fat_offset % params.bytes_per_sector;

        unsigned int sector = (cluster - 2) * params.sectors_per_cluster + rootdir_sector + (params.rootdir_entries * sizeof(fat_entry) / params.bytes_per_sector);

        unsigned char buffer[512];

        if (!last_fat_sector || last_fat_sector != fat_sector) {
            floppy_sector_read(boot_drive, fat_sector, fat_buffer);
        }

        floppy_sector_read(boot_drive, sector, buffer);

        memcpy(addr + cl, buffer, params.bytes_per_sector);
        cl += params.bytes_per_sector;

        last_fat_sector = fat_sector;
        cluster = fat_next_cluster(cluster, fat_buffer, ent_offset);
    }

    return addr;
}

void listfiles() {
    printf("Files on disk: \n");

    fat_entry *f;
    unsigned char buffer[512];

    int rootdir_sector = params.reserved_sectors + params.number_of_fat * params.sectors_per_fat;

    int i;
    for (i = 0; i < params.rootdir_entries * sizeof(fat_entry); i += sizeof(fat_entry)) {
        if (i % 512 == 0) {
            floppy_sector_read(boot_drive, rootdir_sector++, buffer);
        }

        if (buffer[i % 512] == 0) {
            continue;
        }

        buffer2fatentry(&buffer[i % 512], f);

        char fname[9];
        strncpy(fname, f->name, 8);
        printf("  Name: %s.%s\n", fname, f->ext);
        printf("    Size: %d\n", f->size);
        printf("    Cluster: %d\n", f->cluster);
        printf("\n");
    }
}
