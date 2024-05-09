#ifndef USB_H
#define USB_H

#include "pci.h"

enum UHCIRegisters {
    UHCI_REG_USBCMD = 0x00,
    UHCI_REG_USBSTS = 0x02,
    UHCI_REG_USBINTR = 0x04,
    UHCI_REG_FRNUM = 0x06,
    UHCI_REG_FRBASEADD = 0x08,
    UHCI_REG_SOFMOD = 0x0C,
    UHCI_REG_PORTSC1 = 0x10,
    UHCI_REG_PORTSC2 = 0x12,
};

enum UHCICommandRegister {
    UHCI_CMD_RUN = 1 << 0x00,
    UHCI_CMD_HCRESET = 1 << 0x01,
    UHCI_CMD_GRESET = 1 << 0x02,
    UHCI_CMD_GSUSPEND = 1 << 0x03,
    UHCI_CMD_GRESUME = 1 << 0x04,
    UHCI_CMD_SWDBG = 1 << 0x5,
    UHCI_CMD_CF = 1 << 0x6,
    UHCI_CMD_MAXP = 1 << 0x7,
};

enum UHCIStatusRegister {
    UHCI_STS_INT = 1 << 0x00,
    UHCI_STS_ERRINT = 1 << 0x01,
    UHCI_STS_RD = 1 << 0x02,
    UHCI_STS_SE = 1 << 0x03,
    UHCI_STS_PE = 1 << 0x04,
    UHCI_STS_HLT = 1 << 0x05,
};

enum UHCIInterruptEnableRegister {
    UHCI_INT_TOCRC = 1 << 0x00,
    UHCI_INT_RE = 1 << 0x01,
    UHCI_INT_CE = 1 << 0x02,
    UHCI_INT_SP = 1 << 0x03,
};

void usb_init(pci_device *device, pci_header *header, uint8_t bus, uint8_t slot, uint8_t func);

#endif // USB_H
