#ifndef RING3_H
#define RING3_H

#include <stdint.h>

uint32_t _esp;

extern void switch_ring3(void *addr, uint32_t stack);

#endif // RING3_H
