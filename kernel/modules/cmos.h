#ifndef CMOS_H
#define CMOS_H

char read_cmos_register(char reg, char nmi);

int from_bcd(int val);

#endif
