#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

int in_buf_pos = 0;
char in_buf[1024] = {0};

int out_buf_pos = 0;
char out_buf[1024] = {0};

void _init_stdio() {
    in_buf_pos = 0;
    out_buf_pos = 0;
}

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

int _getchar() {
    in_buf_pos = syscall(3, 0, in_buf, 1);
    if (in_buf_pos != 1) {
        return EOF;
    }

    return in_buf[in_buf_pos = 0];
}

int getchar() {
    unsigned char ret = _getchar();
    if (ret <= 0) {
        return -1;
    }

    return ret;
}

int _flush() {
    if (out_buf_pos == 0) {
        return 0;
    }

    int ret = syscall(4, 1, out_buf, out_buf_pos);
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

int _puts(const char *str) {
    while (*str) {
        if (_putchar(*str++) == EOF) {
            return EOF;
        }
    }
}

int puts(const char *str) {
    if (_puts(str) == EOF) {
        return EOF;
    }
    _putchar('\n');

    return _flush();
}

char *gets(char *str) {
    int pos = syscall(3, 0, str, 1024);
    if (pos == 0) {
        return NULL;
    }
    str[pos] = 0;

    return str;
}

int vprintf(const char *format, va_list args) {
    char buf[1024] = {0};

    int ret = vsprintf(buf, format, args);
    if (ret == -1) {
        return -1;
    }

    _puts(buf);
    return _flush();
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
    char curr_mod = 0;
    bool padding = false;
    int padding_size = 0;
    int buf_len;
    bool dec_count = false;
    int dec_count_len = 0;

    while (*format) {
        if (modifier || padding || *format == '%') {
            format++;

            switch (*format) {
                case '%':
                    *str = '%';
                    str++;
                    break;

                case '0':
                    if (!padding && !curr_mod) {
                        curr_mod = *format;
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
                    if (curr_mod == '0') {
                        padding_size *= 10;
                        padding_size += *format - '0';
                    } else if (curr_mod == '.') {
                        dec_count_len *= 10;
                        dec_count_len += *format - '0';
                    }
                    continue;

                case '.':
                    curr_mod = *format;
                    dec_count = true;
                    continue;

                case 'l':
                    modifier++;
                    continue;

                case 'h':
                    modifier--;
                    continue;

                case 'c':
                    *(++str) = (char)va_arg(args, int);
                    break;

                case 's':
                    strcpy(buf, va_arg(args, char *));

                    str = strcat(str, buf);
                    str += strlen(buf);
                    break;

                case 'u':
                    if (modifier == 1) {
                        lutoa(va_arg(args, unsigned long int), buf, 10);
                    } else if (modifier == 0) {
                        utoa(va_arg(args, unsigned int), buf, 10);
                    } else if (modifier == -1) {
                        hutoa((unsigned short int)va_arg(args, unsigned int), buf, 10);
                    }

                    str = strcat(str, buf);
                    str += strlen(buf);
                    break;

                case 'i':
                case 'd':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 10);
                    } else if (modifier == 0) {
                        itoa(va_arg(args, int), buf, 10);
                    } else if (modifier == -1) {
                        htoa((short int)va_arg(args, int), buf, 10);
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
                    } else if (modifier == 0) {
                        itoa(va_arg(args, int), buf, 8);
                    } else if (modifier == -1) {
                        htoa((short int)va_arg(args, int), buf, 8);
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
                    } else if (modifier == 0) {
                        itoa(va_arg(args, int), buf, 2);
                    } else if (modifier == -1) {
                        htoa((short int)va_arg(args, int), buf, 2);
                    }

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    str = strcat(str, buf);
                    str += strlen(buf);
                    break;

                case 'x':
                    if (modifier == 1) {
                        ltoa(va_arg(args, long int), buf, 16);
                    } else if (modifier == 0) {
                        itoa(va_arg(args, int), buf, 16);
                    } else if (modifier == -1) {
                        htoa((short int)va_arg(args, int), buf, 16);
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
                    } else if (modifier == 0) {
                        itoa(va_arg(args, int), buf, 16);
                    } else if (modifier == -1) {
                        htoa((short int)va_arg(args, int), buf, 16);
                    }

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    str = strcat(str, strupr(buf));
                    str += strlen(buf);
                    break;

                case 'f':
                    if (modifier == 1) {
                        lftoa(va_arg(args, double), buf, dec_count ? dec_count_len : 6);
                    } else if (modifier == 0) {
                        ftoa((float)va_arg(args, double), buf, dec_count ? dec_count_len : 6);
                    }

                    buf_len = strlen(buf);
                    if (padding && padding_size > buf_len) {
                        memmove(buf + padding_size - buf_len, buf, buf_len);
                        memset(buf, '0', padding_size - buf_len);
                    }

                    str = strcat(str, buf);
                    str += strlen(buf);

                    break;
            }

            curr_mod = 0;
            modifier = 0;
            padding = false;
            padding_size = 0;
            dec_count = false;
            dec_count_len = 0;
        } else {
            *str = *format;
            str++;
        }

        format++;
    }

    return 0;
}

int scanf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    va_end(args);
    return 0;
}
