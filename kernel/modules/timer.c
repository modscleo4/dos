#include "timer.h"
#include <stdio.h>

time t;
date d;
int timer_ticks = 0;

void getRTC() {
    outb(0x70, 0x8A);
    outb(0x71, 0x20);

    t.second = read_cmos_register(0x00, 1);
    t.minute = read_cmos_register(0x02, 1);
    t.hour = read_cmos_register(0x04, 1);

    d.day = read_cmos_register(0x07, 1);
    d.month = read_cmos_register(0x08, 1);
    d.year = read_cmos_register(0x09, 1);
    dbgprint("Time/date: %d/%d/%d %d:%d:%d\n", d.day, d.month, d.year, t.hour, t.minute, t.second);
}

void timer_phase(int hz) {
    int divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

void timer_handler(registers *r) {
    timer_ticks++;

    if (timer_ticks % 1000 == 0) {
        //
    }
}

void timer_wait(int ms) {
    unsigned long elapsed_ticks = timer_ticks + ms;
    while (timer_ticks < elapsed_ticks) {}
}

void timer_init() {
    getRTC();
    timer_phase(10000);
    irq_install_handler(0, timer_handler);
}
