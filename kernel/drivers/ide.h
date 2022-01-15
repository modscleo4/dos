#ifndef IDE_H
#define IDE_H

#include "ata.h"

#include "iodriver.h"

#define IDE_ATA 0x00
#define IDE_ATAPI 0x01

typedef struct ide_channel_registers {
    unsigned short int base;
    unsigned short int ctrl;
    unsigned short int bmide;
    unsigned char n_ien;
} ide_channel_registers;

typedef struct ide_device {
    unsigned char reserved;
    unsigned char channel;
    unsigned char drive;
    unsigned short int type;
    unsigned short int signature;
    unsigned short int capabilities;
    unsigned int command_sets;
    unsigned int size;
    unsigned char model[41];
} ide_device;

typedef struct prd_table {
    unsigned int addr;
    unsigned short int size;
    unsigned short int reserved: 15;
    bool last: 1;
} __attribute__((packed)) prd_table;

iodriver *ide_init(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);

unsigned char ide_read(unsigned char, unsigned char);

void ide_write(unsigned char, unsigned char, unsigned char);

void ide_read_buffer(unsigned char, unsigned char, unsigned int *, unsigned int);

unsigned char ide_polling(unsigned char, unsigned int);

unsigned char ide_print_error(unsigned int, unsigned char);

void ide_motor_on(unsigned int);

void ide_motor_off(unsigned int);

int ide_do_sector(io_operation, unsigned int, unsigned long int, unsigned int, unsigned char *, bool);

int ide_sector_read(unsigned int, unsigned long int, unsigned char *, bool);

int ide_sector_write(unsigned int, unsigned long int, unsigned char *, bool);

#endif // IDE_H
