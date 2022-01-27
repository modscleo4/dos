#include "ata.h"

#include "../bits.h"
#include "../cpu/irq.h"
#include "ide.h"
#include "../debug.h"
#include <stddef.h>

unsigned static char atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int irq14_c;
int irq15_c;

static void ata0_handler(registers *r) {
    dbgprint("irq14");
    irq14_c++;
}

static void ata1_handler(registers *r) {
    dbgprint("irq14");
    irq15_c++;
}

iodriver *ata_init(unsigned char prog_if, unsigned int BAR0, unsigned int BAR1, unsigned int BAR2, unsigned int BAR3, unsigned int BAR4) {
    irq14_c = 0;
    irq15_c = 0;

    if (ISSET_BIT(prog_if, 0)) { // PCI native mode, controller 0
        //
    } else {
        BAR0 = 0x1F0;
        BAR1 = 0x3F6;
    }

    if (ISSET_BIT(prog_if, 2)) { // PCI native mode, controller 1
        //
    } else {
        BAR2 = 0x170;
        BAR3 = 0x376;
    }

    if (ISSET_BIT(prog_if, 7)) { // DMA
        //
    } else {
        BAR4 = 0x000;
    }

    ata_io = *ide_init(BAR0, BAR1, BAR2, BAR3, BAR4);
    irq_install_handler(14, ata0_handler);
    irq_install_handler(15, ata1_handler);
    return &ata_io;
}

void ata_400ns_delay(unsigned char channel) {
    int i;

    for (i = 0; i < 4; i++) {
        ide_read(channel, ATA_REG_ALTSTATUS);
    }
}

void wait_irq14(void) {
    while (irq14_c <= 0) {}
    irq14_c--;
}

void wait_irq15(void) {
    while (irq15_c <= 0) {}
    irq15_c--;
}
