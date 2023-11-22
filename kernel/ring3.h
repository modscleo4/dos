#ifndef RING3_H
#define RING3_H

#include <stdint.h>
#include "cpu/mmu.h"

extern uint32_t _esp;

extern void switch_ring3(page_directory_table *pdt, void *addr, uint32_t stack);

#endif // RING3_H
