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

FILE *stdin;
FILE *stdout;
FILE *stderr;

int fclose(FILE *);

int fflush(FILE *);

FILE *fopen(const char *, const char *);

FILE *freopen(const char *, const char *, FILE *);

int getchar();

int putchar(char);

int puts(const char *);

char *gets(char *);

int printf(const char *, ...);

int scanf(const char *, ...);

#endif //STDIO_H
