#include <stdbool.h>
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
    char ret;
    while ((ret = keyboard_read()) == -1) { }

    return ret;
}

int putchar(char c) {
    return screen_write(c);
}

int read(char *buf, int size) {
    int i = 0;
    for (i = 0; i < size; i++) {
        buf[i] = getchar();
        if (buf[i] == EOF) {
            break;
        }

        if (buf[i] == '\b') {
            i--;
            if (i < 0) {
                continue;
            }
        }

        putchar(buf[i]);

        if (buf[i] == '\n') {
            break;
        }
    }

    return i;
}

int write(const char *buf, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (putchar(buf[i]) == EOF) {
            break;
        }
    }

    return i;
}

int puts(const char *str) {
    while (*str) {
        if (putchar(*str++) == EOF) {
            return EOF;
        }
    }

    return 1;
}

int vprintf(const char *format, va_list args) {
    char buf[1024] = {0};

    int modifier = 0;
    bool padding = false;
    int padding_size = 0;
    int buf_len;

    while (*format) {
        if (modifier || padding || *format == '%') {
            format++;

            switch (*format) {
                case '%':
                    putchar('%');
                    break;

                case '0':
                    if (!padding) {
                        padding = true;
                        continue;
                    }
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    if (padding) {
                        padding_size *= 10;
                        padding_size += *format - '0';
                    }
                    continue;

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

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    puts(buf);
                    break;

                case 'b':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 2);
                    } else {
                        itoa(va_arg(args, int), buf, 2);
                    }

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    puts(buf);
                    break;

                case 'o':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 8);
                    } else {
                        itoa(va_arg(args, int), buf, 8);
                    }

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    puts(buf);
                    break;

                case 's':
                    strcpy(buf, va_arg(args, char *));

                    puts(buf);
                    break;

                case 'x':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 16);
                    } else {
                        itoa(va_arg(args, int), buf, 16);
                    }

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    puts(buf);
                    break;

                case 'X':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 16);
                    } else {
                        itoa(va_arg(args, int), buf, 16);
                    }

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    puts(strupr(buf));
                    break;
            }

            modifier = 0;
            padding = 0;
            padding_size = 0;
        } else {
            putchar(*format);
        }

        format++;
    }

    return 0;
}

int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    int ret = vprintf(format, args);

    va_end(args);
    return ret;
}

int vsprintf(char *str, const char *format, va_list args) {
    char buf[1024] = {0};

    int modifier = 0;
    bool padding = false;
    int padding_size = 0;
    int buf_len;

        while (*format) {
        if (modifier || *format == '%') {
            format++;

            switch (*format) {
                case '%':
                    *(++str) = '%';
                    break;

                case '0':
                    if (!padding) {
                        padding = true;
                        continue;
                    }
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    if (padding) {
                        padding_size *= 10;
                        padding_size += *format - '0';
                    }
                    continue;

                case 'l':
                    modifier++;
                    continue;

                case 'c':
                    *(++str) = (char)va_arg(args, int);
                    break;

                case 'u':
                    if (modifier == 1) {
                        lutoa(va_arg(args, unsigned long int), buf, 10);
                    } else {
                        utoa(va_arg(args, unsigned int), buf, 10);
                    }

                    str = strcat(str, buf);
                    str += strlen(buf);
                    break;

                case 'i':
                case 'd':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 10);
                    } else {
                        itoa(va_arg(args, int), buf, 10);
                    }

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    str = strcat(str, buf);
                    str += strlen(buf);
                    break;

                case 'o':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 8);
                    } else {
                        itoa(va_arg(args, int), buf, 8);
                    }

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    str = strcat(str, buf);
                    str += strlen(buf);
                    break;

                case 'b':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 2);
                    } else {
                        itoa(va_arg(args, int), buf, 2);
                    }

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    str = strcat(str, buf);
                    str += strlen(buf);
                    break;

                case 's':
                    strcpy(buf, va_arg(args, char *));

                    str = strcat(str, buf);
                    str += strlen(buf);
                    break;

                case 'x':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 16);
                    } else {
                        itoa(va_arg(args, int), buf, 16);
                    }

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    str = strcat(str, buf);
                    str += strlen(buf);
                    break;

                case 'X':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 16);
                    } else {
                        itoa(va_arg(args, int), buf, 16);
                    }

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    str = strcat(str, strupr(buf));
                    str += strlen(buf);
                    break;
            }

            modifier = 0;
        } else {
            *str = *format;
            str++;
        }

        format++;
    }

    return 0;
}

int sprintf(char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);

    int ret = vsprintf(str, format, args);

    va_end(args);
    return ret;
}

int scanf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    va_end(args);
    return 0;
}
