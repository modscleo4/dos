#include "ne2k.h"

#define DEBUG 1

#include <stdlib.h>
#include <string.h>
#include "../../bits.h"
#include "../../debug.h"

ethernet_driver *ne2k_init(pci_device *device, uint8_t bus, uint8_t slot, uint8_t func) {
    dbgprint("Initializing ne2k Ethernet controller\n");

    int iobase = pci_get_bar_address(device->base_address, 0);

    outb(iobase + 0x1F, inb(iobase + 0x1F));
    while ((inb(iobase + 0x07) & 0x80) == 0) { }
    outb(iobase + 0x07, 0xFF);

    unsigned char prom[32];
    outb(iobase + NE2K_REG_COMMAND, (1UL << 5UL) | 1); // page 0, no DMA, stop
    outb(iobase + NE2K_REG_DCR, 0x49);             // set word-wide access
    outb(iobase + NE2K_REG_RBCR0, 0);              // clear the count regs
    outb(iobase + NE2K_REG_RBCR1, 0);
    outb(iobase + NE2K_REG_IMR, 0);                // mask completion IRQ
    outb(iobase + NE2K_REG_ISR, 0xFF);
    outb(iobase + NE2K_REG_RCR, 0x20);             // set to monitor
    outb(iobase + NE2K_REG_TCR, 0x02);             // and loopback mode.
    outb(iobase + NE2K_REG_RBCR0, 32);             // reading 32 bytes
    outb(iobase + NE2K_REG_RBCR1, 0);              // count high
    outb(iobase + NE2K_REG_RSAR0, 0);              // start DMA at 0
    outb(iobase + NE2K_REG_RSAR1, 0);              // start DMA high
    outb(iobase + NE2K_REG_COMMAND, 0x0A);         // start the read

    for (int i = 0; i < 32; i++) {
        prom[i] = inb(iobase + NE2K_REG_DATA);
    }

    // program the PAR0..PAR5 registers to listen for packets to our MAC address!
    outb(iobase + NE2K_REG_COMMAND, (1UL << 6UL) | (1UL << 5UL) | 1); // page 1, no DMA, stop
    for (int i = 0; i < 6; i++) {
        outb(iobase + 0x01 + i, prom[i]);
    }

    outb(iobase + NE2K_REG_COMMAND, (1UL << 5UL) | 1); // page 0, no DMA, stop

    ethernet_driver *driver = malloc(sizeof(ethernet_driver));
    driver->mmiobase = 0;
    driver->iobase = iobase;
    memcpy(&driver->mac, prom, 6);
    driver->ipv4.ip[0] = 0;
    driver->ipv4.ip[1] = 0;
    driver->ipv4.ip[2] = 0;
    driver->ipv4.ip[3] = 0;
    driver->ipv4.netmask[0] = 0;
    driver->ipv4.netmask[1] = 0;
    driver->ipv4.netmask[2] = 0;
    driver->ipv4.netmask[3] = 0;
    driver->ipv4.gateway[0] = 0;
    driver->ipv4.gateway[1] = 0;
    driver->ipv4.gateway[2] = 0;
    driver->ipv4.gateway[3] = 0;
    driver->write = &ne2k_send_packet;
    driver->int_handler = NULL;
    return driver;
}

unsigned int ne2k_send_packet(ethernet_driver *driver, ethernet_packet *packet, size_t data_size) {
    dbgprint("ne2k: packet_length: %d\n", data_size);

    outb(driver->iobase + NE2K_REG_COMMAND, 0x22);
    outb(driver->iobase + NE2K_REG_RBCR0, data_size & 0xFF);
    outb(driver->iobase + NE2K_REG_RBCR1, data_size >> 8);
    outb(driver->iobase + NE2K_REG_ISR, (1UL << 6UL));
    outb(driver->iobase + NE2K_REG_RSAR0, 0x00);
    outb(driver->iobase + NE2K_REG_RSAR1, 0x00);
    outb(driver->iobase + NE2K_REG_COMMAND, 0x12);

    for (int i = 0; i < sizeof(ethernet_header); i++) {
        outb(driver->iobase + NE2K_REG_DATA, ((unsigned char *) packet)[i]);
    }

    for (int i = 0; i < data_size; i++) {
        outb(driver->iobase + NE2K_REG_DATA + sizeof(ethernet_header), ((unsigned char *) packet->data)[i]);
    }

    while ((inb(driver->iobase + NE2K_REG_ISR) & (1UL << 6UL)) == 0) { }

    return 0;
}

void ne2k_receive_packet(ethernet_driver *driver, ethernet_packet *packet, size_t data_size) {
    /*outb(driver->iobase + NE2K_REG_COMMAND, 0x22);
    outb(driver->iobase + NE2K_REG_RBCR0, 0x00);
    outb(driver->iobase + NE2K_REG_RBCR1, 0x00);
    outb(driver->iobase + NE2K_REG_ISR, (1UL << 5UL));
    outb(driver->iobase + NE2K_REG_RSAR0, 0x00);
    outb(driver->iobase + NE2K_REG_RSAR1, 0x40);
    outb(driver->iobase + NE2K_REG_COMMAND, 0x12);

    for (int i = 0; i < sizeof(ethernet_packet); i++) {
        ((unsigned char *) packet)[i] = inb(driver->iobase + NE2K_REG_DATA);
    }

    for (int i = sizeof(ethernet_packet); i < data_size; i++) {
        ((unsigned char *) packet->data)[i - sizeof(ethernet_packet)] = inb(driver->iobase + NE2K_REG_DATA);
    }

    while ((inb(driver->iobase + NE2K_REG_ISR) & (1UL << 5UL)) == 0) { }*/
}
