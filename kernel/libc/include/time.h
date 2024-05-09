#ifndef TIME_H
#define TIME_H

#include <sys/types.h>

extern int       daylight;
extern long int  timezone;
extern char     *tzname[];

struct tm {
    int tm_sec;   // seconds after the minute - [0, 60] including leap second
    int tm_min;   // minutes after the hour - [0, 59]
    int tm_hour;  // hours since midnight - [0, 23]
    int tm_mday;  // day of the month - [1, 31]
    int tm_mon;   // months since January - [0, 11]
    int tm_year;  // years since 1900
    int tm_wday;  // days since Sunday - [0, 6]
    int tm_yday;  // days since January 1 - [0, 365]
    int tm_isdst; // daylight saving time flag
};

#define CLOCKS_PER_SEC 1000
#define CLK_TCK CLOCKS_PER_SEC

struct timespec {
    time_t tv_sec;  // seconds
    long tv_nsec;    // nanoseconds
};

#endif // TIME_H
