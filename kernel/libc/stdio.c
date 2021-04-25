#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../drivers/keyboard.h"
#include "../drivers/screen.h"

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

    putchar((char)ret);
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
    char buf[1024] = {0};

    va_list args;
    va_start(args, format);

    int modifier = 0;

    while (*format) {
        if (modifier || *format == '%') {
            format++;

            switch (*format++) {
                case '%':
                    putchar('%');
                    break;

                case 'l':
                    modifier++;
                    continue;

                case 'c':
                    putchar((char)va_arg(args, int));
                    break;

                case 'u':
                    if (modifier == 1) {
                        lutoa(va_arg(args, unsigned long int), buf, 10);
                    } else {
                        utoa(va_arg(args, unsigned int), buf, 10);
                    }

                    puts(buf);

                    break;

                case 'i':
                case 'd':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 10);
                    } else {
                        itoa(va_arg(args, int), buf, 10);
                    }

                    puts(buf);

                    break;

                case 'o':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 8);
                    } else {
                        itoa(va_arg(args, int), buf, 8);
                    }

                    puts(buf);
                    break;

                case 's':
                    puts(va_arg(args, char *));
                    break;

                case 'x':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 16);
                    } else {
                        itoa(va_arg(args, int), buf, 16);
                    }

                    puts(buf);
                    break;

                case 'X':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 16);
                    } else {
                        itoa(va_arg(args, int), buf, 16);
                    }

                    puts(strupr(buf));
                    break;
            }

            modifier = 0;
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
