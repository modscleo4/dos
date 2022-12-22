#ifndef ATA_H
#define ATA_H

#include "iodriver.h"
#include "pci.h"

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

/*Commands {
    ATAPI_TEST_UNIT_READY = 0x00,
    ATAPI_REQUEST_SENSE = 0x03,
    ATAPI_FORMAT_UNIT = 0x04,
    ATAPI_INQUIRY = 0x12,
    ATAPI_START_STOP_UNIT = 0x1B,
    ATAPI_PREVENT_ALLOW_MEDIUM_REMOVAL = 0x1E,
    ATAPI_READ_FORMAT_CAPACITIES = 0x23,
    ATAPI_READ_CAPACITY = 0x25,
    ATAPI_READ_10 = 0x28,
    ATAPI_WRITE_10 = 0x2A,
    ATAPI_SEEK_10 = 0x2B,
    ATAPI_WRITE_AND_VERIFY_10 = 0x2E,
    ATAPI_VERIFY_10 = 0x2F,
    ATAPI_SYNCHRONIZE_CACHE = 0x35,
    ATAPI_WRITE_BUFFER = 0x3B,
    ATAPI_READ_BUFFER = 0x3C,
    ATAPI_READ_TOC_PMA_ATIP = 0x43,
    ATAPI_GET_CONFIGURATION = 0x46,
    ATAPI_GET_EVENT_STATUS_NOTIFICATION = 0x4A,
    ATAPI_READ_DISC_INFORMATION = 0x51,
    ATAPI_READ_TRACK_INFORMATION = 0x52,
    ATAPI_RESERVE_TRACK = 0x53,
    ATAPI_SEND_OPC_INFORMATION = 0x54,
    ATAPI_MODE_SELECT_10 = 0x55,
    ATAPI_REPAIR_TRACK = 0x58,
    ATAPI_MODE_SENSE_10 = 0x5A,
    ATAPI_CLOSE_TRACK_SESSION = 0x5B,
    ATAPI_READ_BUFFER_CAPACITY = 0x5C,
    ATAPI_SEND_CUE_SHEET = 0x5D,
    ATAPI_REPORT_LUNS = 0xA0,
    ATAPI_BLANK = 0xA1,
    ATAPI_SECURITY_PROTOCOL_IN = 0xA2,
    ATAPI_SEND_KEY = 0xA3,
    ATAPI_REPORT_KEY = 0xA4,
    ATAPI_LOAD_UNLOAD_MEDIUM = 0xA6,
    ATAPI_SET_READ_AHEAD = 0xA7,
    ATAPI_READ_12 = 0xA8,
    ATAPI_WRITE_12 = 0xAA,
    ATAPI_READ_MEDIA_SERIAL_NUMBER_SERVICE_ACTION_IN_12 = 0xAB,
    ATAPI_GET_PERFORMANCE = 0xAC,
    ATAPI_READ_DISC_STRUCTURE = 0xAD,
    ATAPI_SECURITY_PROTOCOL_OUT = 0xB5,
    ATAPI_SET_STREAMING = 0xB6,
    ATAPI_READ_CD_MSF = 0xB9,
    ATAPI_SET_CD_SPEED = 0xBB,
    ATAPI_MECHANISM_STATUS = 0xBD,
    ATAPI_READ_CD = 0xBE,
    ATAPI_SEND_DISC_STRUCTURE = 0xBF
};*/

iodriver ata_io;

iodriver *ata_init(pci_device *device);

void ata_400ns_delay(unsigned char channel);

void ata_wait_irq_primary(void);

void ata_wait_irq_secondary(void);

#endif // ATA_H
