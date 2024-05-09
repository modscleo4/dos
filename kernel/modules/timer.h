#ifndef TIMER_H
#define TIMER_H

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

void rtc_init(void);

void timer_init(void);

void timer_wait(int ms);

void timer_enable_display(void);

void timer_disable_display(void);

#endif //TIMER_H
