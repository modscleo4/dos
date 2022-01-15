#ifndef RING3_H
#define RING3_H

#include <stdint.h>

unsigned long int _esp;

extern void switch_ring3(unsigned long int, unsigned long int);

#endif // RING3_H
