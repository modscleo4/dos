#ifndef NE2K_H
#define NE2K_H

#include "../pci.h"
#include "../ethernet.h"

enum NE2KRegisters {
    NE2K_REG_COMMAND = 0x00,
    NE2K_REG_CLDA0 = 0x01,
    NE2K_REG_CLDA1 = 0x02,
    NE2K_REG_BNRY = 0x03,
    NE2K_REG_TSR = 0x04,
    NE2K_REG_NCR = 0x05,
    NE2K_REG_FIFO = 0x06,
    NE2K_REG_ISR = 0x07,
    NE2K_REG_CRDA0 = 0x08,
    NE2K_REG_CRDA1 = 0x09,
    NE2K_REG_RSR = 0x0C,


    NE2K_REG_PSTART = 0x01,
    NE2K_REG_PSTOP = 0x02,
    NE2K_REG_TPSR = 0x04,
    NE2K_REG_TBCR0 = 0x05,
    NE2K_REG_TBCR1 = 0x06,

    NE2K_REG_RSAR0 = 0x08,
    NE2K_REG_RSAR1 = 0x09,
    NE2K_REG_RBCR0 = 0x0A,
    NE2K_REG_RBCR1 = 0x0B,
    NE2K_REG_RCR = 0x0C,
    NE2K_REG_TCR = 0x0D,
    NE2K_REG_DCR = 0x0E,
    NE2K_REG_IMR = 0x0F,

    NE2K_REG_DATA = 0x10,
};

enum NE2KCommands {
    NE2K_CMD_STOP = 0x01,
    NE2K_CMD_START = 0x02,
    NE2K_CMD_TRANSMIT = 0x04,
    NE2K_CMD_RD0 = 0x08,
    NE2K_CMD_RD1 = 0x10,
    NE2K_CMD_RD2 = 0x20,
    NE2K_CMD_PS0 = 0x40,
    NE2K_CMD_PS1 = 0x80,

    NE2K_CMD_PAGE0 = 0x00,
    NE2K_CMD_PAGE1 = 0x40,
    NE2K_CMD_PAGE2 = 0x80,
};

enum NE2KInterruptStatus {
    NE2K_ISR_PRX = 0x01,
    NE2K_ISR_PTX = 0x02,
    NE2K_ISR_RXE = 0x04,
    NE2K_ISR_TXE = 0x08,
    NE2K_ISR_OVW = 0x10,
    NE2K_ISR_CNT = 0x20,
    NE2K_ISR_RDC = 0x40,
    NE2K_ISR_RST = 0x80,
};

enum NE2KInterruptMask {
    NE2K_IMR_PRXE = 0x01,
    NE2K_IMR_PTXE = 0x02,
    NE2K_IMR_RXEE = 0x04,
    NE2K_IMR_TXEE = 0x08,
    NE2K_IMR_OVWE = 0x10,
    NE2K_IMR_CNTE = 0x20,
    NE2K_IMR_RDCE = 0x40,
};

enum NE2KDataConfiguration {
    NE2K_DCR_WTS = 0x01,
    NE2K_DCR_BOS = 0x02,
    NE2K_DCR_LAS = 0x04,
    NE2K_DCR_LS = 0x08,
    NE2K_DCR_AR = 0x10,
    NE2K_DCR_FT0 = 0x20,
    NE2K_DCR_FT1 = 0x40,
};

enum NE2KTransmitConfiguration {
    NE2K_TCR_CRC = 0x01,
    NE2K_TCR_LB0 = 0x02,
    NE2K_TCR_LB1 = 0x04,
    NE2K_TCR_ATD = 0x08,
    NE2K_TCR_OFST = 0x10,
};

enum NE2KTransmitStatus {
    NE2K_TSR_PTX = 0x01,
    NE2K_TSR_COL = 0x04,
    NE2K_TSR_ABT = 0x08,
    NE2K_TSR_CRS = 0x10,
    NE2K_TSR_FU = 0x20,
    NE2K_TSR_CDH = 0x40,
    NE2K_TSR_OWC = 0x80,
};

enum NE2KReceiveConfiguration {
    NE2K_RCR_SEP = 0x01,
    NE2K_RCR_AR = 0x02,
    NE2K_RCR_AB = 0x04,
    NE2K_RCR_AM = 0x08,
    NE2K_RCR_PRO = 0x10,
    NE2K_RCR_MON = 0x20,
};

enum NE2KReceiveStatus {
    NE2K_RSR_PRX = 0x01,
    NE2K_RSR_CRC = 0x02,
    NE2K_RSR_FAE = 0x04,
    NE2K_RSR_FO = 0x08,
    NE2K_RSR_MPA = 0x10,
    NE2K_RSR_PHY = 0x20,
    NE2K_RSR_DIS = 0x40,
    NE2K_RSR_DFR = 0x80,
};

typedef struct ne2k_receive_buffer {
    uint8_t status;
    uint8_t next_packet;
    uint16_t count;
} ne2k_receive_buffer;

ethernet_driver *ne2k_init(pci_device *device, uint8_t bus, uint8_t slot, uint8_t func);

unsigned int ne2k_send_packet(ethernet_driver *driver, ethernet_packet *packet, size_t packet_length);

void ne2k_receive_packet(ethernet_driver *driver, ethernet_packet *packet, size_t packet_length);

#endif // NE2K_H
