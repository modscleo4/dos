#ifndef CMOS_H
#define CMOS_H

#include <stdint.h>

uint8_t read_cmos_register(uint8_t reg, uint8_t nmi);

int from_bcd(int val);

#endif // CMOS_H
