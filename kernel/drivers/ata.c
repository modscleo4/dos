#include "ata.h"

#define DEBUG 1

#include <stddef.h>
#include "ide.h"
#include "../bits.h"
#include "../debug.h"
#include "../cpu/irq.h"

static uint8_t atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int irq_primary_c;
int irq_secondary_c;

static void ata_primary_handler(registers *r, uint32_t int_no) {
    //dbgprint("irq14");
    irq_primary_c++;
}

static void ata_secondary_handler(registers *r, uint32_t int_no) {
    //dbgprint("irq14");
    irq_secondary_c++;
}

iodriver *ata_init(pci_device *device, uint8_t bus, uint8_t slot, uint8_t func) {
    irq_primary_c = 0;
    irq_secondary_c = 0;

    if (ISSET_BIT(device->header.prog_if, 0)) { // PCI native mode, controller 0
        //
    } else {
        device->base_address[0] = 0x1F0;
        device->base_address[1] = 0x3F6;
    }

    if (ISSET_BIT(device->header.prog_if, 2)) { // PCI native mode, controller 1
        //
    } else {
        device->base_address[2] = 0x170;
        device->base_address[3] = 0x376;
    }

    if (ISSET_BIT(device->header.prog_if, 7)) { // DMA
        //
    } else {
        device->base_address[4] = 0x000;
    }

    ata_io = *ide_init(device);
    irq_install_handler(IRQ_ATA_PRIMARY, ata_primary_handler);
    irq_install_handler(IRQ_ATA_SECONDARY, ata_secondary_handler);
    return &ata_io;
}

void ata_400ns_delay(unsigned char channel) {
    for (int i = 0; i < 4; i++) {
        ide_read(channel, ATA_REG_ALTSTATUS);
    }
}

void ata_wait_irq_primary(void) {
    while (irq_primary_c <= 0) {}
    irq_primary_c--;
}

void ata_wait_irq_secondary(void) {
    while (irq_secondary_c <= 0) {}
    irq_secondary_c--;
}
