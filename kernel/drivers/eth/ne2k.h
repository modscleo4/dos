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

ethernet_driver *ne2k_init(pci_device *device, uint8_t bus, uint8_t slot, uint8_t func);

unsigned int ne2k_send_packet(ethernet_driver *driver, ethernet_packet *packet, size_t packet_length);

void ne2k_receive_packet(ethernet_driver *driver, ethernet_packet *packet, size_t packet_length);

#endif // NE2K_H
