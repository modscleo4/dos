#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include "../cpu/system.h"
#include "../cpu/irq.h"

void timer_wait(int);

void timer_init();

#endif //KERNEL_TIMER_H
