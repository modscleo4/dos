#include "cmos.h"

char read_cmos_register(char reg, char nmi) {
    outb(0x70, (nmi << 7) | reg);
    asm("nop");
    asm("nop");
    asm("nop");
    return inb(0x71);
}
