#ifndef FLOPPY_H
#define FLOPPY_H

#include "../../cpu/system.h"
#include "../iodriver.h"
#include "../pci.h"
#include <stdbool.h>

#define DISK_PARAMETER_ADDRESS 0x000FEFC7

typedef struct chs {
    uint8_t cylinder;
    uint8_t head;
    uint8_t sector;
} chs;

#pragma pack(push, 1)
typedef struct {
    uint8_t steprate_headunload;
    uint8_t headload_ndma;
    uint8_t motor_delay_off; /*specified in clock ticks*/
    uint8_t bytes_per_sector;
    uint8_t sectors_per_track;
    uint8_t gap_length;
    uint8_t data_length; /*used only when bytes per sector == 0*/
    uint8_t format_gap_length;
    uint8_t filler;
    uint8_t head_settle_time; /*specified in milliseconds*/
    uint8_t motor_start_time; /*specified in 1/8 second*/
} floppy_parameters;
#pragma pack(pop)

enum FloppyRegisters {
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

enum FloppyCommands {
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
    FLOPPY_UNLOCK = 20,             // * protect controller params from a reset
    FLOPPY_VERIFY = 22,
    FLOPPY_SCAN_LOW_OR_EQUAL = 25,
    FLOPPY_SCAN_HIGH_OR_EQUAL = 29,
    FLOPPY_LOCK = 0x94
};

enum FloppyMSRFlags {
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
    "2.88MB 3.5in floppy",
};

extern iodriver floppy_io;

iodriver *floppy_init(pci_device *device, uint8_t bus, uint8_t slot, uint8_t func);

void floppy_detect_types(void);

void floppy_configure(iodriver *driver);

uint8_t floppy_version(iodriver *driver);

bool floppy_lock(iodriver *driver, bool lock);

void lba2chs(uint32_t lba, chs *c, floppy_parameters fparams);

void floppy_wait_irq(void);

int floppy_wait_until_ready(iodriver *driver);

uint8_t floppy_recv_byte(iodriver *driver);

int floppy_send_byte(iodriver *driver, uint8_t b);

void floppy_check_interrupt(iodriver *driver, int *st0, int *cylinder);

int floppy_calibrate(iodriver *driver);

int floppy_reset(iodriver *driver);

void floppy_specify(iodriver *driver);

int floppy_seek(iodriver *driver, uint8_t cylinder, uint8_t head);

void floppy_motor_on(iodriver *driver);

void floppy_motor_off(iodriver *driver);

int floppy_do_sector(iodriver *driver, uint32_t lba, uint8_t *buffer, IOOperation direction, bool keepOn);

int floppy_sector_read(iodriver *driver, uint32_t lba, uint8_t *data, bool keepOn);

int floppy_sector_write(iodriver *driver, uint32_t lba, uint8_t *data, bool keepOn);

#endif // FLOPPY_H
