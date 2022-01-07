#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include "../cpu/irq.h"
#include "../cpu/system.h"
#include "cmos.h"

typedef struct time {
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
} time;

typedef struct date {
    unsigned int year;
    unsigned int month;
    unsigned int day;
} date;

void getRTC();

void timer_wait(int);

void timer_init();

#endif //KERNEL_TIMER_H
