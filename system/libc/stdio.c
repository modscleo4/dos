#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    unsigned char ret = syscall(0);
    if (ret == 0) {
        return -1;
    }

    putchar(ret);
    return ret;
}

int out_buf_pos = 0;
char out_buf[1024] = {0};

int _flush() {
    if (out_buf_pos == 0) {
        return 0;
    }

    int ret = syscall(1, out_buf, out_buf_pos);
    out_buf_pos = 0;
    return ret;
}

int _putchar(char c) {
    if (out_buf_pos == 1024) {
        _flush();
    }

    out_buf[out_buf_pos++] = c;
    return 0;
}

int putchar(char c) {
    _putchar(c);
    return _flush();
}

void _puts(const char *str) {
    while (*str) {
        if (_putchar(*str++) == EOF) {
            return EOF;
        }
    }
}

int puts(const char *str) {
    _puts(str);
    _putchar('\n');

    return _flush();
}

int vprintf(const char *format, va_list args) {
    char buf[1024] = {0};

    int modifier = 0;

    while (*format) {
        if (modifier || *format == '%') {
            format++;

            switch (*format++) {
                case '%':
                    _putchar('%');
                    break;

                case 'l':
                    modifier++;
                    continue;

                case 'c':
                    _putchar((char)va_arg(args, int));
                    break;

                case 'u':
                    if (modifier == 1) {
                        lutoa(va_arg(args, unsigned long int), buf, 10);
                    } else {
                        utoa(va_arg(args, unsigned int), buf, 10);
                    }

                    _puts(buf);

                    break;

                case 'i':
                case 'd':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 10);
                    } else {
                        itoa(va_arg(args, int), buf, 10);
                    }

                    _puts(buf);

                    break;

                case 'o':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 8);
                    } else {
                        itoa(va_arg(args, int), buf, 8);
                    }

                    _puts(buf);
                    break;

                case 's':
                    _puts(va_arg(args, char *));
                    break;

                case 'x':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 16);
                    } else {
                        itoa(va_arg(args, int), buf, 16);
                    }

                    _puts(buf);
                    break;

                case 'X':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 16);
                    } else {
                        itoa(va_arg(args, int), buf, 16);
                    }

                    _puts(strupr(buf));
                    break;
            }

            modifier = 0;
        } else {
            _putchar(*format++);
        }
    }

    _flush();

    return 0;
}

int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    int ret = vprintf(format, args);

    va_end(args);
    return ret;
}

int scanf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    va_end(args);
    return 0;
}
