#ifndef CMOS_H
#define CMOS_H

#include "../bits.h"

char read_cmos_register(char, char);

int from_bcd(int);

#endif
