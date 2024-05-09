#ifndef IDE_H
#define IDE_H

#include "ata.h"

#include "../iodriver.h"
#include "../pci.h"

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
    uint32_t sector_size;
    uint8_t model[41];
} ide_device;

#pragma pack(push, 1)
typedef struct prd_table {
    uint32_t addr;
    uint16_t size;
    uint16_t reserved: 15;
    bool last: 1;
} prd_table;
#pragma pack(pop)

iodriver *ide_init(pci_device *device);

int ide_reset(iodriver *driver);

uint8_t ide_read(uint8_t channel, uint8_t reg);

void ide_write(uint8_t channel, uint8_t reg, uint8_t data);

void ide_read_buffer(uint8_t channel, uint8_t reg, uint16_t *buffer, uint16_t quads);

int ide_send_atapi_command(uint8_t device, uint16_t length, uint8_t command[12]);

uint8_t ide_polling(uint8_t channel, uint16_t advanced_check);

uint8_t ide_print_error(iodriver *driver, uint8_t err);

void ide_motor_on(iodriver *driver);

void ide_motor_off(iodriver *driver);

int ide_do_sector(IOOperation direction, iodriver *driver, uint32_t lba, uint16_t number_of_sectors, uint8_t *buffer, bool keepOn);

int ide_sector_read(iodriver *driver, uint32_t lba, uint8_t *buffer, bool keepOn);

int ide_sector_write(iodriver *driver, uint32_t lba, uint8_t *buffer, bool keepOn);

int ide_search_for_drive(int boot_drive);

#endif // IDE_H
