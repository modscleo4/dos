#include "timer.h"

#include "../bits.h"
#include "../debug.h"
#include "../cpu/irq.h"
#include "../cpu/pic.h"
#include "../cpu/system.h"
#include "cmos.h"

time t;
date d;
int timer_ticks = 0;

void rtc_init(void) {
    outb(0x70, 0x8A);
    outb(0x71, 0x20);

    t.second = from_bcd(read_cmos_register(0x00, 1));
    t.minute = from_bcd(read_cmos_register(0x02, 1));
    t.hour = from_bcd(read_cmos_register(0x04, 1));

    d.day = from_bcd(read_cmos_register(0x07, 1));
    d.month = from_bcd(read_cmos_register(0x08, 1));
    d.year = from_bcd(read_cmos_register(0x09, 1));
    dbgprint("Time/date: %02d/%02d/%02d %02d:%02d:%02d\n", d.day, d.month, d.year, t.hour, t.minute, t.second);
}

void timer_phase(int hz) {
    int divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

static void timer_irq_handler(registers *r) {
    timer_ticks += 10;

    if (timer_ticks % 1000 == 0) {

    }
}

void timer_wait(int ms) {
    unsigned long elapsed_ticks = timer_ticks + ms;
    while (timer_ticks < elapsed_ticks) {}
}

void timer_init(void) {
    rtc_init();
    timer_phase(100);
    irq_install_handler(IRQ_PIT, timer_irq_handler);
}
