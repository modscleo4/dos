#ifndef FLOPPY_H
#define FLOPPY_H

#include "../bits.h"
#include "../modules/cmos.h"
#include "../cpu/irq.h"
#include "../modules/timer.h"
#include "fat.h"
#include <stdlib.h>
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

enum FloppyRegisters {
    STATUS_REGISTER_A = 0x000, // read-only
    STATUS_REGISTER_B = 0x001, // read-only
    DIGITAL_OUTPUT_REGISTER = 0x002,
    TAPE_DRIVE_REGISTER = 0x003,
    MAIN_STATUS_REGISTER = 0x004,     // read-only
    DATARATE_SELECT_REGISTER = 0x004, // write-only
    DATA_FIFO = 0x005,
    DIGITAL_INPUT_REGISTER = 0x007,        // read-only
    CONFIGURATION_CONTROL_REGISTER = 0x007 // write-only
};

enum FloppyCommands {
    READ_TRACK = 2, // generates IRQ6
    SPECIFY = 3,    // * set drive parameters
    SENSE_DRIVE_STATUS = 4,
    WRITE_DATA = 5,      // * write to the disk
    READ_DATA = 6,       // * read from the disk
    RECALIBRATE = 7,     // * seek to cylinder 0
    SENSE_INTERRUPT = 8, // * ack IRQ6, get status of last command
    WRITE_DELETED_DATA = 9,
    READ_ID = 10, // generates IRQ6
    READ_DELETED_DATA = 12,
    FORMAT_TRACK = 13, // *
    DUMPREG = 14,
    SEEK = 15,    // * seek both heads to cylinder X
    VERSION = 16, // * used during initialization, once
    SCAN_EQUAL = 17,
    PERPENDICULAR_MODE = 18, // * used during initialization, once, maybe
    CONFIGURE = 19,          // * set controller parameters
    LOCK = 20,               // * protect controller params from a reset
    VERIFY = 22,
    SCAN_LOW_OR_EQUAL = 25,
    SCAN_HIGH_OR_EQUAL = 29
};

enum FloppyMSRFlags {
    MSR_MRQ = 0x80,
    MSR_DIO = 0x40,
    MSR_NON_DMA = 0x20,
    MSR_BUSY = 0x10,
    MSR_ACTD = 0x08,
    MSR_ACTC = 0x04,
    MSR_ACTB = 0x02,
    MSR_ACTA = 0x01

};

static const char *drive_types[6] = {
    "No floppy drive.",
    "360KB 5.25in floppy",
    "1.2MB 5.25in floppy",
    "720KB 3.5in floppy",
    "1.44MB 3.5in floppy",
    "2.88MB 3.5in floppy"};

typedef enum {
    floppy_direction_read = 1,
    floppy_direction_write = 2
} floppy_direction;

int init_floppy();

void detect_floppy_types();

void lba2chs(unsigned long int, chs *, floppy_parameters);

int floppy_recv_byte(unsigned int);

int floppy_send_byte(unsigned int, unsigned char);

void floppy_check_interrupt(unsigned int, int *, int *);

int floppy_calibrate(unsigned int);

int ResetFloppy(unsigned int);

int floppy_seek(unsigned int, unsigned char, unsigned char);

void floppy_motor_on(unsigned int);

void floppy_motor_off(unsigned int);

void floppy_handler(registers *);

static void floppy_dma_init(floppy_direction, unsigned char *);

int floppy_do_sector(unsigned int, unsigned long int, unsigned char *, floppy_direction, bool);

int floppy_sector_read(unsigned int, unsigned long int, unsigned char *, bool);

int floppy_sector_write(unsigned int, unsigned long int, unsigned char *, bool);

#endif //FLOPPY_H
