#include "ne2k.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdlib.h>
#include <string.h>
#include "../../bits.h"
#include "../../debug.h"

static void ne2k_int_handler(ethernet_driver *driver) {
    dbgprint("ne2k: interrupt\n");
}

static inline uint8_t ne2k_io_read(ethernet_driver *driver, enum NE2KRegisters reg) {
    return inb(driver->iobase + reg);
}

static inline void ne2k_io_write(ethernet_driver *driver, enum NE2KRegisters reg, uint8_t value) {
    outb(driver->iobase + reg, value);
}

ethernet_driver *ne2k_init(pci_device *device, uint8_t bus, uint8_t slot, uint8_t func) {
    dbgprint("Initializing ne2k Ethernet controller\n");

    int iobase = pci_get_bar_address(device->base_address, 0);

    outb(iobase + 0x1F, inb(iobase + 0x1F));
    while ((inb(iobase + 0x07) & 0x80) == 0) { }
    outb(iobase + 0x07, 0xFF);

    uint8_t prom[32];
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

    for (int i = 0; i < 6; i++) {
        prom[i] = inb(iobase + NE2K_REG_DATA);
    }

    // program the PAR0..PAR5 registers to listen for packets to our MAC address!
    outb(iobase + NE2K_REG_COMMAND, (1UL << 6UL) | (1UL << 5UL) | 1); // page 1, no DMA, stop
    for (int i = 0; i < 6; i++) {
        outb(iobase + 0x01 + i, prom[i]);
    }

    outb(iobase + NE2K_REG_COMMAND, (1UL << 5UL) | 1); // page 0, no DMA, stop

    ethernet_driver *driver = malloc(sizeof(ethernet_driver));
    driver->lock = spinlock_init();
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
    driver->int_handler = &ne2k_int_handler;
    driver->int_enable = NULL;
    driver->int_disable = NULL;
    return driver;
}

static inline void ne2k_set_rsar(ethernet_driver *driver, uint16_t address) {
    ne2k_io_write(driver, NE2K_REG_RSAR0, address & 0xFF);
    ne2k_io_write(driver, NE2K_REG_RSAR1, address >> 8);
}

static inline void ne2k_set_rbcr(ethernet_driver *driver, uint16_t size) {
    ne2k_io_write(driver, NE2K_REG_RBCR0, size & 0xFF);
    ne2k_io_write(driver, NE2K_REG_RBCR1, size >> 8);
}

static inline void ne2k_set_tbcr(ethernet_driver *driver, uint16_t size) {
    ne2k_io_write(driver, NE2K_REG_TBCR0, size & 0xFF);
    ne2k_io_write(driver, NE2K_REG_TBCR1, size >> 8);
}

unsigned int ne2k_send_packet(ethernet_driver *driver, ethernet_packet *packet, size_t data_size) {
    dbgprint("ne2k: packet_length: %d\n", data_size);

    spinlock_lock(driver->lock);

    ne2k_io_write(driver, NE2K_REG_DCR, 0x49);
    //ne2k_io_write(driver, NE2K_REG_COMMAND, NE2K_CMD_RD2 | NE2K_CMD_START);
    //ne2k_io_write(driver, NE2K_REG_RBCR0, (data_size + sizeof(ethernet_header)) & 0xFF);
    //ne2k_io_write(driver, NE2K_REG_RBCR1, (data_size + sizeof(ethernet_header)) >> 8);
    ne2k_set_rbcr(driver, data_size + sizeof(ethernet_header));
    ne2k_set_rsar(driver, 16 * 1024);
    ne2k_io_write(driver, NE2K_REG_ISR, NE2K_ISR_RDC);
    //ne2k_io_write(driver, NE2K_REG_RSAR0, ((16 * 1024) & 0xFF));
    //ne2k_io_write(driver, NE2K_REG_RSAR1, ((16 * 1024) >> 8));
    ne2k_io_write(driver, NE2K_REG_COMMAND, NE2K_CMD_RD1 | NE2K_CMD_START);

    for (int i = 0; i < sizeof(ethernet_header); i++) {
        ne2k_io_write(driver, NE2K_REG_DATA, ((uint8_t *) packet)[i]);
    }

    for (int i = 0; i < data_size; i++) {
        ne2k_io_write(driver, NE2K_REG_DATA, ((uint8_t *) packet->data)[i]);
    }

    while (!ISSET_BIT_INT(ne2k_io_read(driver, NE2K_REG_ISR), NE2K_ISR_RDC)) { asm volatile ("hlt"); }

    ne2k_io_write(driver, NE2K_REG_TPSR, (16 * 1024) / 256);

    //ne2k_io_write(driver, NE2K_REG_TBCR0, (data_size + sizeof(ethernet_header)) & 0xFF);
    //ne2k_io_write(driver, NE2K_REG_TBCR1, (data_size + sizeof(ethernet_header)) >> 8);
    ne2k_set_tbcr(driver, data_size + sizeof(ethernet_header));

    ne2k_io_write(driver, NE2K_REG_COMMAND, NE2K_CMD_RD2 | NE2K_CMD_TRANSMIT | NE2K_CMD_START);

    while (!ISSET_BIT_INT(ne2k_io_read(driver, NE2K_REG_COMMAND), (1UL << 2UL))) { asm volatile ("hlt"); }

    spinlock_unlock(driver->lock);

    return 0;
}

void ne2k_receive_packet(ethernet_driver *driver, ethernet_packet *packet, size_t data_size) {
    spinlock_lock(driver->lock);

    ne2k_io_write(driver, NE2K_REG_COMMAND, 0x22);
    ne2k_io_write(driver, NE2K_REG_RBCR0, 0x00);
    ne2k_io_write(driver, NE2K_REG_RBCR1, 0x00);
    ne2k_io_write(driver, NE2K_REG_ISR, (1UL << 5UL));
    ne2k_io_write(driver, NE2K_REG_RSAR0, 0x00);
    ne2k_io_write(driver, NE2K_REG_RSAR1, 0x40);
    ne2k_io_write(driver, NE2K_REG_COMMAND, 0x12);

    for (int i = 0; i < sizeof(ethernet_packet); i++) {
        ((uint8_t *) packet)[i] = ne2k_io_read(driver, NE2K_REG_DATA);
    }

    for (int i = sizeof(ethernet_packet); i < data_size; i++) {
        ((uint8_t *) packet->data)[i - sizeof(ethernet_packet)] = ne2k_io_read(driver, NE2K_REG_DATA);
    }

    while ((ne2k_io_read(driver, NE2K_REG_ISR) & (1UL << 5UL)) == 0) { }

    spinlock_unlock(driver->lock);
}
