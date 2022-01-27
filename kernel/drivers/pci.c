#include "pci.h"

#include "ata.h"
#include "floppy.h"
#include "../bits.h"
#include "../debug.h"
#include <string.h>

void pci_init(void) {
    pci_discover_devices();
}

unsigned short int pci_read_word(unsigned int bus, unsigned int slot, unsigned int func, unsigned short int offset) {
    unsigned int address = 0x80000000 | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return (unsigned short int) (inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF;
}

void pci_write_word(unsigned int bus, unsigned int slot, unsigned int func, unsigned short int offset, unsigned int value) {
    unsigned int address = 0x80000000 | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

void pci_read_header(unsigned int bus, unsigned int slot, unsigned int func, pci_header *header) {
    for (int i = 0; i < sizeof(pci_header); i += sizeof(unsigned short int)) {
        unsigned short int v = pci_read_word(bus, slot, func, i);
        memcpy(((unsigned char *) header) + i, &v, sizeof(unsigned short int));
    }
}

static void pci_fix_bar(unsigned int bar[], unsigned int x) {
    // Retrieve the actual address of the BAR

    // If the BAR is a memory BAR (bit 0 is not set)
    if (!ISSET_BIT(bar[x], 0)) {
        unsigned int type = (bar[x] > 1) & 0x2;
        // If the BAR is a 32-bit BAR
        if (type == 0x00) {
            // Retrieve the 32-bit BAR
            bar[x] &= 0xFFFFFFF0;
        } else if (type == 0x02) {
            // Retrieve the 64-bit BAR
            bar[x] = (bar[x] & 0xFFFFFFF0) | ((bar[x + 1] & 0xFFFFFFF0) << 32);
        }
    } else {
        bar[x] &= 0xFFFFFFFC;
    }
}

void pci_read_device(unsigned int bus, unsigned int slot, unsigned int func, pci_header *header, pci_device *device) {
    // Assuming the header is already read
    memcpy(&device->header, header, sizeof(pci_header));

    for (int i = sizeof(pci_header); i < sizeof(pci_device); i += sizeof(unsigned short int)) {
        unsigned short int v = pci_read_word(bus, slot, func, i);
        memcpy(((unsigned char *)device) + i, &v, sizeof(unsigned short int));
    }

    for (int i = 0; i < 6; i++) {
        pci_fix_bar(device->base_address, i);
    }
}

void pci_read_pci_bridge(unsigned int bus, unsigned int slot, unsigned int func, pci_header *header, pci_pci_bridge *bridge) {
    // Assuming the header is already read
    memcpy(&bridge->header, header, sizeof(pci_header));

    for (int i = sizeof(pci_header); i < sizeof(pci_pci_bridge); i += sizeof(unsigned short)) {
        unsigned short v = pci_read_word(bus, slot, func, i);
        memcpy(((unsigned char *) bridge) + i, &v, sizeof(unsigned short));
    }

    for (int i = 0; i < 2; i++) {
        pci_fix_bar(bridge->base_address, i);
    }
}

void pci_read_cardbus_bridge(unsigned int bus, unsigned int slot, unsigned int func, pci_header *header, pci_cardbus_bridge *bridge) {
    // Assuming the header is already read
    memcpy(&bridge->header, header, sizeof(pci_header));

    for (int i = sizeof(pci_header); i < sizeof(pci_cardbus_bridge); i += sizeof(unsigned short)) {
        unsigned short v = pci_read_word(bus, slot, func, i);
        memcpy(((unsigned char *) bridge) + i, &v, sizeof(unsigned short));
    }
}

static void pci_device_found(unsigned char bus, unsigned char slot, unsigned char func, pci_header *header) {
    if (header->header_type.type == 0x00) {
        dbgprint("PCI device found: %x:%x:%x\n", bus, slot, func);
        pci_device device;
        pci_read_device(bus, slot, func, header, &device);
        if (header->class == 0x01) {
            dbgprint("\tMass Storage Controller: %x\n", header->subclass);
            // Set BARs
            if (header->subclass == 0x01) {
                ata_init(device.header.prog_if, device.base_address[0], device.base_address[1], device.base_address[2], device.base_address[3], device.base_address[4]);
            } else if (header->subclass == 0x02) {
                floppy_init(device.header.prog_if, device.base_address[0], device.base_address[1], device.base_address[2], device.base_address[3], device.base_address[4]);
            }
        }
        //dbgprint("\tVendor: %x\n", header.vendor);
        //dbgprint("\tDevice: %x\n", header.device);
        //dbgprint("\tClass: %x\n", header.class);
        //dbgprint("\tSubclass: %x\n", header.subclass);
        //dbgprint("\tProgIF: %x\n", header.prog_if);
    }
}

void pci_check_device(unsigned char bus, unsigned char device) {
    unsigned char func = 0;

    pci_header header;
    pci_read_header(bus, device, func, &header);
    if (header.vendor == 0xFFFF) {
        return;
    }

    pci_check_function(bus, device, func, &header);
    if (header.header_type.multi_func) {
        // Multi function device
        for (func = 1; func < 8; func++) {
            pci_read_header(bus, device, func, &header);
            if (header.vendor != 0xFFFF) {
                pci_device_found(bus, device, func, &header);

                pci_check_function(bus, device, func, &header);
            }
        }
    } else {
        pci_device_found(bus, device, func, &header);
    }
}

void pci_check_bus(unsigned char bus) {
    unsigned char device;

    for (device = 0; device < 32; device++) {
        pci_check_device(bus, device);
    }
}

void pci_check_function(unsigned char bus, unsigned char device, unsigned char func, pci_header *h) {
    pci_header header;
    if (!h) {
        pci_read_header(bus, device, func, &header);
    } else {
        header = *h;
    }

    if (header.class == 0x06 && header.subclass == 0x04) {
        pci_pci_bridge bridge;
        memcpy(&bridge, &header, sizeof(pci_header));
        pci_read_pci_bridge(bus, device, func, &header, &bridge);
        pci_check_bus(bridge.secondary_bus);
    }
}

void pci_discover_devices(void) {
    unsigned char func;
    unsigned char bus;

    // Get header of first device
    pci_header header;
    pci_read_header(0, 0, 0, &header);

    if (!header.header_type.multi_func) {
        pci_check_bus(0);
    } else {
        for (func = 0; func < 8; func++) {
            pci_read_header(0, 0, func, &header);
            if (header.vendor != 0xFFFF) {
                break;
            }

            bus = func;
            pci_check_bus(bus);
        }
    }
}
