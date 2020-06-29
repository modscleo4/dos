#include "timer.h"
#include <stdio.h>

int timer_ticks = 0;

void timer_phase(int hz) {
    int divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

void timer_handler(registers *r) {
    timer_ticks++;

    if (timer_ticks % 100 == 0) {

    }
}

void timer_wait(int ticks) {
    unsigned long elapsed_ticks = timer_ticks + ticks;
    while (timer_ticks < elapsed_ticks);
}

void timer_init() {
    timer_phase(100);
    irq_install_handler(0, timer_handler);
}
