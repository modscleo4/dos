#ifndef ATA_H
#define ATA_H

#include "../iodriver.h"
#include "../pci.h"

enum ATAIORegisters {
    ATA_REG_DATA = 0x000,
    ATA_REG_ERROR = 0x001,
    ATA_REG_FEATURES = 0x001,
    ATA_REG_SECCOUNT0 = 0x002,
    ATA_REG_LBA0 = 0x003,
    ATA_REG_LBA1 = 0x004,
    ATA_REG_LBA2 = 0x005,
    ATA_REG_HDDEVSEL = 0x006,
    ATA_REG_COMMAND = 0x007,
    ATA_REG_STATUS = 0x007,
    ATA_REG_SECCOUNT1 = 0x008,
    ATA_REG_LBA3 = 0x009,
    ATA_REG_LBA4 = 0x00A,
    ATA_REG_LBA5 = 0x00B,
    ATA_REG_CONTROL = 0x00C,
    ATA_REG_ALTSTATUS = 0x00C,
    ATA_REG_DEVADDRESS = 0x00D
};

enum ATACRRegisters {
    ATA_CR_ALTERNATE = 0x000,
    ATA_CR_DEVICE = 0x000,
    ATA_CR_HEAD = 0x001,
};

enum ATAErrorRegisters {
    ATA_ERR_AMNT = 0x00,
    ATA_ERR_TKZNF = 0x01,
    ATA_ERR_ABRT = 0x04,
    ATA_ERR_MCR = 0x08,
    ATA_ERR_IDNF = 0x10,
    ATA_ERR_MC = 0x20,
    ATA_ERR_UNC = 0x40,
    ATA_ERR_BBK = 0x80
};

enum ATAStatusRegisters {
    ATA_SR_BSY = 0x80,
    ATA_SR_DRDY = 0x40,
    ATA_SR_DF = 0x20,
    ATA_SR_DSC = 0x10,
    ATA_SR_DRQ = 0x08,
    ATA_SR_CORR = 0x04,
    ATA_SR_IDX = 0x02,
    ATA_SR_ERR = 0x01
};

enum ATACommands {
    ATA_CMD_READ_PIO = 0x20,
    ATA_CMD_READ_PIO_EXT = 0x24,
    ATA_CMD_READ_DMA = 0xC8,
    ATA_CMD_READ_DMA_EXT = 0x25,
    ATA_CMD_WRITE_PIO = 0x30,
    ATA_CMD_WRITE_PIO_EXT = 0x34,
    ATA_CMD_WRITE_DMA = 0xCA,
    ATA_CMD_WRITE_DMA_EXT = 0x35,
    ATA_CMD_CACHE_FLUSH = 0xE7,
    ATA_CMD_CACHE_FLUSH_EXT = 0xEA,
    ATA_CMD_PACKET = 0xA0,
    ATA_CMD_IDENTIFY_PACKET = 0xA1,
    ATA_CMD_IDENTIFY = 0xEC
};

enum ATAIdentify {
    ATA_ID_DEVICETYPE = 0,
    ATA_ID_CYLINDERS = 2,
    ATA_ID_HEADS = 6,
    ATA_ID_SECTORS = 12,
    ATA_ID_SERIAL = 20,
    ATA_ID_MODEL = 54,
    ATA_ID_CAPABILITIES = 98,
    ATA_ID_FIELDVALID = 106,
    ATA_ID_MAX_LBA = 120,
    ATA_ID_COMMANDSETS = 164,
    ATA_ID_MAX_LBA_EXT = 200
};

enum ATABusMasterRegisters {
    ATA_BMR_COMMAND = 0x000,
    ATA_BMR_STATUS = 0x002,
    ATA_BMR_PRDT = 0x004,
};

#define ATA_MASTER 0x00
#define ATA_SLAVE 0x01

#define ATA_PRIMARY 0x00
#define ATA_SECONDARY 0x01

#define ATA_READ 0x00
#define ATA_WRITE 0x01

extern iodriver ata_io;

iodriver *ata_init(pci_device *device, uint8_t bus, uint8_t slot, uint8_t func);

void ata_400ns_delay(unsigned char channel);

void ata_wait_irq_primary(void);

void ata_wait_irq_secondary(void);

int ata_search_for_drive(int boot_drive);

#endif // ATA_H
