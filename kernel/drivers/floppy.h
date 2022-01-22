#ifndef FLOPPY_H
#define FLOPPY_H

#include "../cpu/system.h"
#include "iodriver.h"
#include <stdbool.h>

#define DISK_PARAMETER_ADDRESS 0x000FEFC7

typedef struct chs {
    unsigned char cylinder;
    unsigned char head;
    unsigned char sector;
} chs;

typedef struct {
    unsigned char steprate_headunload;
    unsigned char headload_ndma;
    unsigned char motor_delay_off; /*specified in clock ticks*/
    unsigned char bytes_per_sector;
    unsigned char sectors_per_track;
    unsigned char gap_length;
    unsigned char data_length; /*used only when bytes per sector == 0*/
    unsigned char format_gap_length;
    unsigned char filler;
    unsigned char head_settle_time; /*specified in milliseconds*/
    unsigned char motor_start_time; /*specified in 1/8 second*/
} __attribute__((packed)) floppy_parameters;

#define FLOPPY_PRIMARY_BASE 0x03F0
#define FLOPPY_SECONDARY_BASE 0x0370
#define FLOPPY_TERTIARY_BASE 0x0360

enum Floppy_Registers {
    FLOPPY_STATUS_REGISTER_A = 0x000, // read-only
    FLOPPY_STATUS_REGISTER_B = 0x001, // read-only
    FLOPPY_DIGITAL_OUTPUT_REGISTER = 0x002,
    FLOPPY_TAPE_DRIVE_REGISTER = 0x003,
    FLOPPY_MAIN_STATUS_REGISTER = 0x004,     // read-only
    FLOPPY_DATARATE_SELECT_REGISTER = 0x004, // write-only
    FLOPPY_DATA_FIFO = 0x005,
    FLOPPY_DIGITAL_INPUT_REGISTER = 0x007,        // read-only
    FLOPPY_CONFIGURATION_CONTROL_REGISTER = 0x007 // write-only
};

enum Floppy_Commands {
    FLOPPY_READ_TRACK = 2, // generates IRQ6
    FLOPPY_SPECIFY = 3,    // * set drive parameters
    FLOPPY_SENSE_DRIVE_STATUS = 4,
    FLOPPY_WRITE_DATA = 5,      // * write to the disk
    FLOPPY_READ_DATA = 6,       // * read from the disk
    FLOPPY_RECALIBRATE = 7,     // * seek to cylinder 0
    FLOPPY_SENSE_INTERRUPT = 8, // * ack IRQ6, get status of last command
    FLOPPY_WRITE_DELETED_DATA = 9,
    FLOPPY_READ_ID = 10, // generates IRQ6
    FLOPPY_READ_DELETED_DATA = 12,
    FLOPPY_FORMAT_TRACK = 13, // *
    FLOPPY_DUMPREG = 14,
    FLOPPY_SEEK = 15,    // * seek both heads to cylinder X
    FLOPPY_VERSION = 16, // * used during initialization, once
    FLOPPY_SCAN_EQUAL = 17,
    FLOPPY_PERPENDICULAR_MODE = 18, // * used during initialization, once, maybe
    FLOPPY_CONFIGURE = 19,          // * set controller parameters
    FLOPPY_UNLOCK = 20,               // * protect controller params from a reset
    FLOPPY_VERIFY = 22,
    FLOPPY_SCAN_LOW_OR_EQUAL = 25,
    FLOPPY_SCAN_HIGH_OR_EQUAL = 29,
    FLOPPY_LOCK = 0x94
};

enum Floppy_MSR_Flags {
    FLOPPY_MSR_MRQ = 0x80,
    FLOPPY_MSR_DIO = 0x40,
    FLOPPY_MSR_NON_DMA = 0x20,
    FLOPPY_MSR_BUSY = 0x10,
    FLOPPY_MSR_ACTD = 0x08,
    FLOPPY_MSR_ACTC = 0x04,
    FLOPPY_MSR_ACTB = 0x02,
    FLOPPY_MSR_ACTA = 0x01
};

static const char *drive_types[6] = {
    "No floppy drive.",
    "360KB 5.25in floppy",
    "1.2MB 5.25in floppy",
    "720KB 3.5in floppy",
    "1.44MB 3.5in floppy",
    "2.88MB 3.5in floppy"
};

iodriver *floppy_init(unsigned char, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);

void floppy_detect_types(void);

void floppy_configure(unsigned int);

unsigned char floppy_version(unsigned int);

bool floppy_lock(unsigned int, bool);

void lba2chs(unsigned long int, chs *, floppy_parameters);

void wait_irq6(void);

int floppy_wait_until_ready(unsigned int);

int floppy_recv_byte(unsigned int);

int floppy_send_byte(unsigned int, unsigned char);

void floppy_check_interrupt(unsigned int, int *, int *);

int floppy_calibrate(unsigned int);

int floppy_reset(unsigned int);

void floppy_specify(unsigned int);

int floppy_seek(unsigned int, unsigned char, unsigned char);

void floppy_motor_on(unsigned int);

void floppy_motor_off(unsigned int);

static void floppy_dma_init(io_operation, unsigned char *);

int floppy_do_sector(unsigned int, unsigned long int, unsigned char *, io_operation, bool);

int floppy_sector_read(unsigned int, unsigned long int, unsigned char *, bool);

int floppy_sector_write(unsigned int, unsigned long int, unsigned char *, bool);

iodriver floppy_io;

#endif //FLOPPY_H
