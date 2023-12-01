#ifndef IDE_H
#define IDE_H

#include "ata.h"

#include "iodriver.h"
#include "pci.h"

#define IDE_ATA 0x00
#define IDE_ATAPI 0x01

typedef struct ide_channel_registers {
    uint16_t base;
    uint16_t ctrl;
    uint16_t bmide;
    uint8_t n_ien;
} ide_channel_registers;

typedef struct ide_device {
    uint8_t reserved;
    uint8_t channel;
    uint8_t drive;
    uint16_t type;
    uint16_t signature;
    uint16_t capabilities;
    uint32_t command_sets;
    uint32_t size;
    uint8_t model[41];
} ide_device;

typedef struct prd_table {
    uint32_t addr;
    uint16_t size;
    uint16_t reserved: 15;
    bool last: 1;
} __attribute__((packed)) prd_table;

iodriver *ide_init(pci_device *device);

unsigned char ide_read(unsigned char channel, unsigned char reg);

void ide_write(unsigned char channel, unsigned char reg, unsigned char data);

void ide_read_buffer(unsigned char channel, unsigned char reg, unsigned int *buffer, unsigned int quads);

unsigned char ide_polling(unsigned char channel, unsigned int advanced_check);

unsigned char ide_print_error(iodriver *driver, unsigned char err);

void ide_motor_on(iodriver *driver);

void ide_motor_off(iodriver *driver);

int ide_do_sector(IOOperation direction, iodriver *driver, unsigned long int lba, unsigned int number_of_sectors, uint8_t *buffer, bool keepOn);

int ide_sector_read(iodriver *driver, unsigned long int lba, uint8_t *buffer, bool keepOn);

int ide_sector_write(iodriver *driver, unsigned long int lba, uint8_t *buffer, bool keepOn);

#endif // IDE_H
