#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

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

void timer_wait(int);

void timer_init(void);

#endif //KERNEL_TIMER_H
