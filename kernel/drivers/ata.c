#include "ata.h"

#include "../bits.h"
#include "../cpu/irq.h"
#include "ide.h"
#include <stddef.h>

unsigned static char atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int irq14_c;
int irq15_c;

static unsigned int bar0;
static unsigned int bar1;
static unsigned int bar2;
static unsigned int bar3;
static unsigned int bar4;

static void ata0_handler(registers *r) {
    dbgprint("irq14");
    irq14_c++;
}

static void ata1_handler(registers *r) {
    dbgprint("irq14");
    irq15_c++;
}

void ata_load_bars(unsigned char prog_if, unsigned int BAR0, unsigned int BAR1, unsigned int BAR2, unsigned int BAR3, unsigned int BAR4) {
    if (ISSET_BIT(prog_if, 0)) { // PCI native mode, controller 0
        bar0 = BAR0;
        bar1 = BAR1;
    } else {
        bar0 = 0x1F0;
        bar1 = 0x3F6;
    }

    if (ISSET_BIT(prog_if, 2)) { // PCI native mode, controller 1
        bar2 = BAR2;
        bar3 = BAR3;
    } else {
        bar2 = 0x170;
        bar3 = 0x376;
    }

    if (ISSET_BIT(prog_if, 7)) { // DMA
        bar4 = BAR4;
    } else {
        bar4 = 0x000;
    }
}

iodriver *ata_init(unsigned int boot_drive) {
    irq14_c = 0;
    irq15_c = 0;

    iodriver *driver = ide_init(bar0, bar1, bar2, bar3, bar4);
    irq_install_handler(14, ata0_handler);
    irq_install_handler(15, ata1_handler);
    driver->device = boot_drive;
    return driver;
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
