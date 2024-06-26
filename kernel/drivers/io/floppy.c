#include "floppy.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include "../../bits.h"
#include "../../cpu/irq.h"
#include "../../cpu/mmu.h"
#include "../../cpu/pic.h"
#include "../../debug.h"
#include "../../modules/bitmap.h"
#include "../../modules/cmos.h"
#include "../../modules/timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int drives[2] = {0, 0};
floppy_parameters floppy;
int floppy_motor_state[2] = {0, 0}; /* 0: OFF, 1: ON; 2: WAIT */

static int irq_c;

unsigned int FLOPPY_PRIMARY_BASE = 0x3F0;
unsigned int FLOPPY_SECONDARY_BASE = 0x370;

iodriver floppy_io;

static void floppy_handler(registers *r, uint32_t int_no) {
    irq_c++;
}

iodriver *floppy_init(pci_device *device, uint8_t bus, uint8_t slot, uint8_t func) {
    dbgprint("Initializing floppy driver\n");

    irq_c = 0;
    uint8_t *io_buffer = (uint8_t *)malloc_align(512, BITMAP_PAGE_SIZE);

    memcpy(&floppy, (uint8_t *)DISK_PARAMETER_ADDRESS, sizeof(floppy_parameters));

    dbgprint("Installing IRQ handler %d for floppy drive\n", device ? device->interrupt_line : IRQ_FLOPPY);
    irq_install_handler(device ? device->interrupt_line : IRQ_FLOPPY, floppy_handler);

    floppy_detect_types();

    uint8_t version = floppy_version(0);
    if (version != 0x90 && version != 0xFF) { // 0xFF is Bochs' version
        dbgprint("Floppy drive #%d is not supported: version %x.\n", 0, version);
        return NULL;
    }

    if (device) {
        FLOPPY_PRIMARY_BASE = device->base_address[0] & 0xFFFFFFFC;
        FLOPPY_SECONDARY_BASE = device->base_address[1] & 0xFFFFFFFC;
    }

    floppy_io.device       = -1;
    floppy_io.io_buffer    = io_buffer;
    floppy_io.sector_size  = floppy.bytes_per_sector;
    floppy_io.reset        = &floppy_reset;
    floppy_io.start        = &floppy_motor_on;
    floppy_io.stop         = &floppy_motor_off;
    floppy_io.read_sector  = &floppy_sector_read;
    floppy_io.write_sector = &floppy_sector_write;
    return &floppy_io;
}

void floppy_detect_types(void) {
    uint8_t c = read_cmos_register(0x10, 1);

    drives[0] = c >> 4U;
    drives[1] = c & 0xFU;

    dbgprint("Floppy 0: %d - %s\n", drives[0], drive_types[drives[0]]);
    dbgprint("Floppy 1: %d - %s\n", drives[1], drive_types[drives[1]]);
}

void floppy_configure(iodriver *driver) {
    bool implied_seek_enable = false;
    bool fifo_disable = false;
    bool drive_polling_mode_disable = true;
    uint8_t thresh_val = 8;
    uint8_t precomp_val = 0;

    floppy_send_byte(driver, FLOPPY_CONFIGURE);
    floppy_send_byte(driver, 0x00);
    floppy_send_byte(driver, (implied_seek_enable << 6) | (fifo_disable << 5) | (drive_polling_mode_disable << 4) | (thresh_val & 0xF));
    floppy_send_byte(driver, precomp_val);
}

uint8_t floppy_version(iodriver *driver) {
    floppy_send_byte(driver, FLOPPY_VERSION);
    return floppy_recv_byte(driver);
}

bool floppy_lock(iodriver *driver, bool lock) {
    floppy_send_byte(driver, FLOPPY_LOCK | lock << 7);
    return ISSET_BIT(floppy_recv_byte(driver), 4);
}

void lba2chs(uint32_t lba, chs *c, floppy_parameters fparams) {
    c->cylinder = lba / (2 * fparams.sectors_per_track);
    c->head = ((lba % (2 * fparams.sectors_per_track)) / fparams.sectors_per_track);
    c->sector = ((lba % (2 * fparams.sectors_per_track)) % fparams.sectors_per_track + 1);
}

void floppy_wait_irq(void) {
    while (irq_c <= 0) {
        asm volatile("hlt");
    }
    irq_c--;
}

int floppy_wait_until_ready(iodriver *driver) {
    int base = (driver->device <= 1) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;

    for (int counter = 0; counter < 10000; counter++) {
        int status;
        if ((status = inb(base + FLOPPY_MAIN_STATUS_REGISTER)) & FLOPPY_MSR_MRQ) {
            return status;
        }
    }

    return -1;
}

uint8_t floppy_recv_byte(iodriver *driver) {
    int base = (driver->device <= 1) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;

    for (int i = 0; i < 600; i++) {
        char in = inb(base + FLOPPY_MAIN_STATUS_REGISTER);
        if (ISSET_BIT_INT(in, FLOPPY_MSR_MRQ)) {
            return inb(base + FLOPPY_DATA_FIFO);
        }
    }

    return -1;
}

int floppy_send_byte(iodriver *driver, uint8_t b) {
    int base = (driver->device <= 1) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;

    for (int i = 0; i < 600; i++) {
        char in = inb(base + FLOPPY_MAIN_STATUS_REGISTER);
        if (ISSET_BIT_INT(in, FLOPPY_MSR_MRQ) && !ISSET_BIT_INT(in, FLOPPY_MSR_DIO)) {
            outb(base + FLOPPY_DATA_FIFO, b);
            return 0;
        }
    }

    return -1;
}

void floppy_check_interrupt(iodriver *driver, int *st0, int *cylinder) {
    floppy_send_byte(driver, FLOPPY_SENSE_INTERRUPT);
    *st0 = floppy_recv_byte(driver);
    *cylinder = floppy_recv_byte(driver);
}

int floppy_calibrate(iodriver *driver) {
    int i;
    int st0 = 0;
    int cylinder = -1;

    floppy_motor_on(driver);

    for (i = 0; i < 10; i++) {
        floppy_send_byte(driver, FLOPPY_RECALIBRATE);
        floppy_send_byte(driver, driver->device);

        floppy_wait_irq();
        floppy_check_interrupt(driver, &st0, &cylinder);
        dbgprint("st0: %d, cylinder: %d\n", st0, cylinder);

        if (0xC0 & st0) {
            static const char *status[] = {0, "error", "invalid", "drive"};
            dbgprint("status = %s\n", status[st0 >> 6]);
            continue;
        }

        if (!cylinder) {
            floppy_motor_off(driver);
            return 0;
        }
    }

    dbgprint("10 retries exhausted\n");
    floppy_motor_off(driver);

    return -1;
}

int floppy_reset(iodriver *driver) {
    int base = (driver->device <= 1) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;
    uint8_t dor = inb(base + FLOPPY_DIGITAL_INPUT_REGISTER);
    outb(base + FLOPPY_DIGITAL_OUTPUT_REGISTER, 0x00);
    outb(base + FLOPPY_DIGITAL_OUTPUT_REGISTER, ENABLE_BIT(dor, 2));

    floppy_wait_irq();

    for (int i = 0; i < 4; i++) {
        int st0 = 0;
        int cylinder = -1;
        floppy_check_interrupt(driver, &st0, &cylinder);
    }

    // Transfer speed: 500kb/s
    outb(base + FLOPPY_CONFIGURATION_CONTROL_REGISTER, 0x00);

    floppy_specify(driver);

    if (floppy_calibrate(driver)) {
        return -1;
    }

    switch (drives[driver->device]) {
        case 1:
            driver->size = 360 * 1024;
            break;

        case 2:
            driver->size = 1200 * 1024;
            break;

        case 3:
            driver->size = 720 * 1024;
            break;

        case 4:
            driver->size = 1440 * 1024;
            break;

        case 5:
            driver->size = 2880 * 1024;
            break;
    }

    return 0;
}

void floppy_specify(iodriver *driver) {
    unsigned int data_rate = 500000;
    unsigned int SRT = 16 - (6 * data_rate / 500000);
    unsigned int HUT = 30 * data_rate / 1000000;
    unsigned int HLT = 1;
    unsigned int NDMA = 0;
    floppy_send_byte(driver, FLOPPY_SPECIFY);
    floppy_send_byte(driver, (SRT << 4) | HUT);
    floppy_send_byte(driver, (HLT << 1 | NDMA));
}

int floppy_seek(iodriver *driver, uint8_t cylinder, uint8_t head) {
    int st0 = 0;
    int cyl = -1;

    floppy_motor_on(driver);

    for (int i = 0; i < 10; i++) {
        floppy_send_byte(driver, FLOPPY_SEEK);
        floppy_send_byte(driver, head << 2);
        floppy_send_byte(driver, cylinder);

        floppy_wait_irq();
        floppy_check_interrupt(driver, &st0, &cyl);

        if (0xC0 & st0) {
            static const char *status[] = {"normal", "error", "invalid", "drive"};
            dbgprint("status = %s\n", status[st0 >> 6]);
            continue;
        }

        if (cyl == cylinder) {
            floppy_motor_off(driver);
            return 0;
        }
    }

    dbgprint("10 retries exhausted\n");
    floppy_motor_off(driver);

    return -1;
}

void floppy_motor_on(iodriver *driver) {
    int base = (driver->device <= 1) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;
    if (floppy_motor_state[driver->device] != 1) {
        outb(base + FLOPPY_DIGITAL_OUTPUT_REGISTER, 0x1C);
        timer_wait(50);
    }

    floppy_motor_state[driver->device] = 1;
}

void floppy_motor_off(iodriver *driver) {
    int base = (driver->device <= 1) ? FLOPPY_PRIMARY_BASE : FLOPPY_SECONDARY_BASE;
    if (floppy_motor_state[driver->device] != 0) {
        outb(base + FLOPPY_DIGITAL_OUTPUT_REGISTER, 0x0C);
        timer_wait(50);
    }
    floppy_motor_state[driver->device] = 0;
}

static int floppy_dma_init(IOOperation direction, uint8_t *buffer) {
    union {
        uint8_t b[4];
        uint32_t l;
    } addr, count;

    addr.l = (uint32_t)mmu_get_physical_address((uintptr_t)buffer);
    count.l = (uint32_t)511;

    if ((addr.l >> 24) || (count.l >> 16) || (((addr.l & 0xffff) + count.l) >> 16)) {
        dbgprint("buffer problem\n");
        return -1;
    }

    uint8_t mode;

    switch (direction) {
        case IO_READ:
            mode = 0x46;
            break;

        case IO_WRITE:
            mode = 0x4a;
            break;

        default:
            dbgprint("invalid direction");
            return -1;
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

    return 0;
}

int floppy_do_sector(iodriver *driver, uint32_t lba, uint8_t *buffer, IOOperation direction, bool keepOn) {
    dbgprint("%s sector %d\n", direction == IO_READ ? "Reading" : "Writing", lba);
    uint8_t command;
    static const int flags = 0xC0;

    chs c;
    lba2chs(lba, &c, floppy);

    switch (direction) {
        case IO_READ:
            command = FLOPPY_READ_DATA | flags;
            break;

        case IO_WRITE:
            command = FLOPPY_WRITE_DATA | flags;
            break;
    }

    if (floppy_seek(driver, c.cylinder, c.head)) {
        return -1;
    }

    for (int i = 0; i < 20; i++) {
        floppy_motor_on(driver);
        if (floppy_dma_init(direction, buffer)) {
            return -3;
        }
        timer_wait(floppy.head_settle_time);

        floppy_send_byte(driver, command);
        floppy_send_byte(driver, (c.head << 2) | driver->device);
        floppy_send_byte(driver, c.cylinder);
        floppy_send_byte(driver, c.head);
        floppy_send_byte(driver, c.sector);
        floppy_send_byte(driver, floppy.bytes_per_sector);
        floppy_send_byte(driver, floppy.sectors_per_track);
        floppy_send_byte(driver, floppy.gap_length);
        floppy_send_byte(driver, floppy.data_length);

        floppy_wait_irq();

        uint8_t st0, st1, st2, rcy, rhe, rse, bps;
        st0 = floppy_recv_byte(driver);
        st1 = floppy_recv_byte(driver);
        st2 = floppy_recv_byte(driver);
        rcy = floppy_recv_byte(driver);
        rhe = floppy_recv_byte(driver);
        rse = floppy_recv_byte(driver);
        bps = floppy_recv_byte(driver);

        int error = 0;

        if (st0 & 0xC0) {
            static const char *status[] = {0, "error", "invalid command", "drive not ready"};
            dbgprint("status = %s\n", status[st0 >> 6]);
            error = 1;
        }

        if (st1 & 0x80) {
            dbgprint("end of cylinder\n");
            error = 1;
        }

        if (st0 & 0x08) {
            dbgprint("drive not ready\n");
            error = 1;
        }

        if (st1 & 0x20) {
            dbgprint("CRC error\n");
            error = 1;
        }

        if (st1 & 0x10) {
            dbgprint("controller timeout\n");
            error = 1;
        }

        if (st1 & 0x04) {
            dbgprint("no data found\n");
            error = 1;
        }

        if ((st1 | st2) & 0x01) {
            dbgprint("no address mark found\n");
            error = 1;
        }

        if (st2 & 0x40) {
            dbgprint("deleted address mark\n");
            error = 1;
        }

        if (st2 & 0x20) {
            dbgprint("CRC error in data\n");
            error = 1;
        }

        if (st2 & 0x10) {
            dbgprint("wrong cylinder\n");
            error = 1;
        }

        if (st2 & 0x04) {
            dbgprint("uPD765 sector not found\n");
            error = 1;
        }

        if (st2 & 0x02) {
            dbgprint("bad cylinder\n");
            error = 1;
        }

        if (bps != 0x2) {
            dbgprint("wanted 512B/sector, got %d", (1 << (bps + 7)));
            error = 1;
        }

        if (st1 & 0x02) {
            dbgprint("not writable\n");
            error = 2;
        }

        if (!error) {
            if (!keepOn) {
                floppy_motor_off(driver);
            }

            return 0;
        }

        if (error > 1) {
            dbgprint("not retrying..\n");
            floppy_motor_off(driver);

            return -2;
        }
    }

    dbgprint("20 retries exhausted\n");
    floppy_motor_off(driver);

    return -1;
}

int floppy_sector_read(iodriver *driver, uint32_t lba, uint8_t *data, bool keepOn) {
    return floppy_do_sector(driver, lba, data, IO_READ, keepOn);
}

int floppy_sector_write(iodriver *driver, uint32_t lba, uint8_t *data, bool keepOn) {
    return floppy_do_sector(driver, lba, data, IO_WRITE, keepOn);
}
