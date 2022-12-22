#include "cmos.h"

#include "../bits.h"

uint8_t read_cmos_register(uint8_t reg, uint8_t nmi) {
    outb(0x70, (nmi << 7) | reg);
    return inb(0x71);
}

int from_bcd(int val) {
    return ((val / 16) * 10) + (val % 16);
}
