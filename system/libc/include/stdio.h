#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

#define EOF (-1)

#ifndef NULL
#define NULL (void *)0
#endif

typedef struct iobuf {
    char *ptr;
    int cnt;
    char *base;
    int flag;
    int file;
    int charbuf;
    int bufsiz;
} FILE;

#define _IOREAD 1
#define _IOWRT  2
#define _IORW   0x0080

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int fclose(FILE *);

int fflush(FILE *);

FILE *fopen(const char *, const char *);

FILE *freopen(const char *, const char *, FILE *);

int getchar(void);

int putchar(char);

int puts(const char *);

char *gets(char *);

int vprintf(const char *, va_list);

int printf(const char *, ...);

int vsprintf(char *, const char *, va_list);

int sprintf(char *, const char *, ...);

int scanf(const char *, ...);

#endif //STDIO_H
