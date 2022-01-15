#include "floppy.h"

#include "../bits.h"
#include "../cpu/irq.h"
#include "../cpu/pic.h"
#include "../debug.h"
#include "../modules/cmos.h"
#include "../modules/timer.h"
#include "fat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int drives[2] = {0, 0};
floppy_parameters floppy;
int floppy_motor_state[2] = {0, 0}; /* 0: OFF, 1: ON; 2: WAIT */

int irq6_c;
unsigned char io_buffer[512];

static void floppy_handler(registers *r) {
    irq6_c++;
}

iodriver *floppy_init(unsigned int boot_drive) {
    irq6_c = 0;

    memcpy(&floppy, (unsigned char *)DISK_PARAMETER_ADDRESS, sizeof(floppy_parameters));
    irq_install_handler(IRQ_FLOPPY, floppy_handler);

    floppy_detect_types();
    dbgprint("Boot drive is #%d\n", boot_drive);
    dbgprint("  Head settle time: %d\n", floppy.head_settle_time);

    static iodriver driver;
    driver.device = boot_drive;
    driver.io_buffer = io_buffer;
    driver.reset = &floppy_reset;
    driver.start = &floppy_motor_on;
    driver.stop = &floppy_motor_off;
    driver.read_sector = &floppy_sector_read;
    driver.write_sector = &floppy_sector_write;
    return &driver;
}

void floppy_detect_types(void) {
    unsigned char c = read_cmos_register(0x10, 1);

    drives[0] = c >> 4U;
    drives[1] = c & 0xFU;

    dbgprint("Floppy 0: %d - %s\n", drives[0], drive_types[drives[0]]);
    dbgprint("Floppy 1: %d - %s\n", drives[1], drive_types[drives[1]]);
}

void lba2chs(unsigned long int lba, chs *c, floppy_parameters fparams) {
    c->cylinder = lba / (2 * fparams.sectors_per_track);
    c->head = ((lba % (2 * fparams.sectors_per_track)) / fparams.sectors_per_track);
    c->sector = ((lba % (2 * fparams.sectors_per_track)) % fparams.sectors_per_track + 1);
}

void wait_irq6(void) {
    while (irq6_c <= 0) {}
    irq6_c--;
}

int floppy_wait_until_ready(unsigned int drive) {
    int base = (drive == 0) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;

    for(int counter = 0; counter < 10000; counter++) {
        int status;
        if((status = inb(base + FLOPPY_MAIN_STATUS_REGISTER)) & FLOPPY_MSR_MRQ) {
            return status;
        }
    }

    return -1;
}


int floppy_recv_byte(unsigned int drive) {
    int base = (drive <= 1) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;

    for (int i = 0; i < 600; i++) {
        char in = inb(base + FLOPPY_MAIN_STATUS_REGISTER);
        if (ISSET_BIT_INT(in, FLOPPY_MSR_MRQ)) {
            return inb(base + FLOPPY_DATA_FIFO);
        }
    }

    return -1;
}

int floppy_send_byte(unsigned int drive, unsigned char b) {
    int base = (drive <= 1) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;

    for (int i = 0; i < 600; i++) {
        char in = inb(base + FLOPPY_MAIN_STATUS_REGISTER);
        if (ISSET_BIT_INT(in, FLOPPY_MSR_MRQ) && !ISSET_BIT_INT(in, FLOPPY_MSR_DIO)) {
            outb(base + FLOPPY_DATA_FIFO, b);
            return 0;
        }
    }

    return -1;
}

void floppy_check_interrupt(unsigned int drive, int *st0, int *cylinder) {
    floppy_send_byte(drive, FLOPPY_SENSE_INTERRUPT);
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
        floppy_send_byte(drive, FLOPPY_RECALIBRATE);
        floppy_send_byte(drive, drive);

        wait_irq6();
        floppy_check_interrupt(drive, &st0, &cylinder);
        dbgprint("%s: st0: %d, cylinder: %d\n", fn, st0, cylinder);

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

int floppy_reset(unsigned int drive) {
    int base = (drive == 0) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;
    outb(base + FLOPPY_DIGITAL_OUTPUT_REGISTER, 0x00);
    outb(base + FLOPPY_DIGITAL_OUTPUT_REGISTER, 0x0C);

    wait_irq6();

    for (int i = 0; i < 4; i++) {
        int st0 = 0;
        int cylinder = -1;
        floppy_check_interrupt(drive, &st0, &cylinder);
    }

    // Transfer speed: 500kb/s
    outb(base + FLOPPY_CONFIGURATION_CONTROL_REGISTER, 0x00);

    floppy_specify(drive);

    if (floppy_calibrate(drive)) {
        return -1;
    }

    return 0;
}

void floppy_specify(unsigned int drive) {
    unsigned int data_rate = 500000;
    unsigned int SRT = 16 - (6 * data_rate / 500000);
    unsigned int HUT = 30 * data_rate / 1000000;
    unsigned int HLT = 1;
    unsigned int NDMA = 0;
    floppy_send_byte(drive, FLOPPY_SPECIFY);
    floppy_send_byte(drive, (SRT << 4) | HUT);
    floppy_send_byte(drive, (HLT << 1 | NDMA));
}

int floppy_seek(unsigned int drive, unsigned char cylinder, unsigned char head) {
    const char *fn = "floppy_seek";
    int st0 = 0;
    int cyl = -1;

    floppy_motor_on(drive);

    for (int i = 0; i < 10; i++) {
        floppy_send_byte(drive, FLOPPY_SEEK);
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
        outb(base + FLOPPY_DIGITAL_OUTPUT_REGISTER, 0x1C);
        timer_wait(50);
    }

    floppy_motor_state[drive] = 1;
}

void floppy_motor_off(unsigned int drive) {
    int base = (drive == 0) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;
    if (floppy_motor_state[drive] != 0) {
        outb(base + FLOPPY_DIGITAL_OUTPUT_REGISTER, 0x0C);
        timer_wait(50);
    }
    floppy_motor_state[drive] = 0;
}

static void floppy_dma_init(io_operation direction, unsigned char *buffer) {
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
        case io_read:
            mode = 0x46;
            break;

        case io_write:
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

int floppy_do_sector(unsigned int drive, unsigned long int lba, unsigned char *buffer, io_operation direction, bool keepOn) {
    const char *fn = "floppy_do_sector";
    unsigned char command;
    static const int flags = 0xC0;

    chs c;
    lba2chs(lba, &c, floppy);

    switch (direction) {
        case io_read:
            command = FLOPPY_READ_DATA | flags;
            break;

        case io_write:
            command = FLOPPY_WRITE_DATA | flags;
            break;

        default:
            dbgprint("%s: invalid direction", fn);
            asm("hlt");
            return 0;
    }

    if (floppy_seek(drive, c.cylinder, c.head)) {
        return -1;
    }

    for (int i = 0; i < 20; i++) {
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
            if (!keepOn) {
                floppy_motor_off(drive);
            }

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

int floppy_sector_read(unsigned int drive, unsigned long int lba, unsigned char *data, bool keepOn) {
    return floppy_do_sector(drive, lba, data, io_read, keepOn);
}

int floppy_sector_write(unsigned int drive, unsigned long int lba, unsigned char *data, bool keepOn) {
    return floppy_do_sector(drive, lba, data, io_write, keepOn);
}
