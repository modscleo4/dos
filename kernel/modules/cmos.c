#include "cmos.h"

#include "../bits.h"

char read_cmos_register(char reg, char nmi) {
    outb(0x70, (nmi << 7) | reg);
    return inb(0x71);
}

int from_bcd(int val) {
    return ((val / 16) * 10) + (val % 16);
}
