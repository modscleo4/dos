#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"

int fclose(FILE *stream) {
    return 0;
}

int fflush(FILE *stream) {
    return 0;
}

FILE *fopen(const char *filename, const char *mode) {
    return NULL;
}

FILE *freopen(const char *filename, const char *mode, FILE *stream) {
    return NULL;
}

int getchar() {
    unsigned char ret = keyboard_read();
    if (ret == 0) {
        return -1;
    }

    putchar(ret);
    return ret;
}

int putchar(char c) {
    return screen_write(c);
}

int puts(const char *str) {
    while (*str) {
        if (putchar(*str++) == EOF) {
            return EOF;
        }
    }

    return 1;
}

int printf(const char *format, ...) {
    unsigned int i;
    char buf[1024];

    va_list args;
    va_start(args, format);

    while (*format != '\0') {
        if (*format == '%') {
            format++;

            switch (*format++) {
                case '%':
                    putchar('%');
                    break;

                case 'c':
                    i = va_arg(args, int);
                    putchar(i);
                    break;

                case 'd':
                    i = va_arg(args, int);
                    puts(itoa(i, buf, 10));
                    break;

                case 'o':
                    i = va_arg(args, int);
                    puts(itoa(i, buf, 8));
                    break;

                case 's':
                    puts(va_arg(args, char *));
                    break;

                case 'x':
                    i = va_arg(args, int);
                    puts(itoa(i, buf, 16));
                    break;

                case 'X':
                    i = va_arg(args, int);
                    puts(strupr(itoa(i, buf, 16)));
                    break;
            }
        } else {
            putchar(*format++);
        }
    }

    va_end(args);
    return 0;
}

int scanf(const char *format, ...) {
    va_list args;
    va_start(args, format);


    va_end(args);
    return 0;
}
