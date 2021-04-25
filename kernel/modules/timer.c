#include "timer.h"
#include <stdio.h>

time t;
date d;
int timer_ticks = 0;

void getRTC() {
    outb(0x70, 0x8A);
    outb(0x71, 0x20);

    t.second = inb(0x00);
    t.minute = inb(0x02);
    t.hour = inb(0x04);

    d.day = inb(0x07);
    d.month = inb(0x08);
    d.year = inb(0x32) << 2 | inb(0x09);
    printf("Time/date: %d/%d/%d %d:%d:%d\n", d.day, d.month, d.year, t.hour, t.minute, t.second);
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
    timer_phase(1000);
    irq_install_handler(0, timer_handler);
}
