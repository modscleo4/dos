#include "timer.h"

#include <stdio.h>
#include "cmos.h"
#include "../bits.h"
#include "../debug.h"
#include "../cpu/irq.h"
#include "../cpu/pic.h"
#include "../cpu/system.h"
#include "../drivers/screen.h"

time t;
date d;
int timer_ticks = 0;

static void read_rtc(void) {
    t.second = from_bcd(read_cmos_register(0x00, 1));
    t.minute = from_bcd(read_cmos_register(0x02, 1));
    t.hour = from_bcd(read_cmos_register(0x04, 1));

    d.day = from_bcd(read_cmos_register(0x07, 1));
    d.month = from_bcd(read_cmos_register(0x08, 1));
    d.year = from_bcd(read_cmos_register(0x09, 1));

    int century = from_bcd(read_cmos_register(0x32, 1));
    if (century == 0) {
        century = d.year >= 90 ? 19 : 20;
    }
    d.year += century * 100;
}

void rtc_init(void) {
    outb(0x70, 0x8A);
    outb(0x71, 0x20);

    read_rtc();
    dbgprint("Time/date: %02d/%02d/%04d %02d:%02d:%02d\n", d.day, d.month, d.year, t.hour, t.minute, t.second);
}

void timer_phase(int hz) {
    int divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

static void timer_irq_handler(registers *r, uint32_t int_no) {
    timer_ticks += 10;

    if (timer_ticks % 1000 == 0) {
        read_rtc();
        char color;
        int x;
        int y;
        color = screen_getcolor();
        screen_getxy(&x, &y);

        screen_gotoxy(80-19, 0);
        screen_setcolor(COLOR_RED << 4 | COLOR_GRAY);
        printf("%02d/%02d/%04d %02d:%02d:%02d", d.day, d.month, d.year, t.hour, t.minute, t.second);

        screen_gotoxy(x, y);
        screen_setcolor(color);
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
