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

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int fclose(FILE *stream);

int fflush(FILE *stream);

FILE *fopen(const char *filename, const char *mode);

FILE *freopen(const char *filename, const char *mode, FILE *stream);

int getchar(void);

int putchar(char c);

int read(int fd, void *buf, int size);

int write(int fd, const void *buf, int size);

int puts(const char *str);

int vprintf(const char *format, va_list args);

int printf(const char *format, ...);

int vsprintf(char *str, const char *format, va_list args);

int sprintf(char *str, const char *format, ...);

int scanf(const char *format, ...);

#endif //STDIO_H
