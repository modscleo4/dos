#include "ata.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stddef.h>
#include "ide.h"
#include "../../bits.h"
#include "../../debug.h"
#include "../../cpu/irq.h"

iodriver ata_io;

static const uint8_t atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

volatile static int irq_primary_c;
volatile static int irq_secondary_c;

static void ata_primary_handler(registers *r, uint32_t int_no) {
    //dbgprint("irq14");
    irq_primary_c++;
}

static void ata_secondary_handler(registers *r, uint32_t int_no) {
    //dbgprint("irq15");
    irq_secondary_c++;
}

iodriver *ata_init(pci_device *device, uint8_t bus, uint8_t slot, uint8_t func) {
    irq_primary_c = 0;
    irq_secondary_c = 0;

    uint32_t irq_primary = IRQ_ATA_PRIMARY;
    uint32_t irq_secondary = IRQ_ATA_SECONDARY;

    if (ISSET_BIT(device->header.prog_if, 0)) { // PCI native mode, controller 0
        irq_primary = device->interrupt_line;
    } else {
        device->base_address[0] = 0x1F0;
        device->base_address[1] = 0x3F6;
    }

    if (ISSET_BIT(device->header.prog_if, 2)) { // PCI native mode, controller 1
        irq_secondary = device->interrupt_line;
    } else {
        device->base_address[2] = 0x170;
        device->base_address[3] = 0x376;
    }

    if (ISSET_BIT(device->header.prog_if, 7)) { // DMA
        device->header.command.bus_master = 1;
        device->header.command.memory_space = 1;
        pci_write_word(bus, slot, func, 0x04, *(uint16_t *)&device->header.command);
    } else {
        device->base_address[4] = 0x000;
    }

    ata_io = *ide_init(device);
    irq_install_handler(irq_primary, ata_primary_handler);
    irq_install_handler(irq_secondary, ata_secondary_handler);
    return &ata_io;
}

void ata_400ns_delay(unsigned char channel) {
    for (int i = 0; i < 4; i++) {
        ide_read(channel, ATA_REG_ALTSTATUS);
    }
}

void ata_wait_irq_primary(void) {
    while (irq_primary_c <= 0) { asm volatile("hlt"); }
    irq_primary_c--;
}

void ata_wait_irq_secondary(void) {
    while (irq_secondary_c <= 0) { asm volatile("hlt"); }
    irq_secondary_c--;
}

int ata_search_for_drive(int boot_drive) {
    // Assume IDE

    return ide_search_for_drive(boot_drive);
}
