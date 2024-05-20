#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

FILE *stdin = NULL;
FILE *stdout = NULL;
FILE *stderr = NULL;

void _init_stdio(void) {
    char *stdin_buf = malloc(1024);
    char *stdout_buf = malloc(1024);
    char *stderr_buf = malloc(1024);

    stdin = malloc(sizeof(FILE));
    stdin->file = STDIN_FILENO;
    stdin->flag = _IOREAD;
    stdin->bufsiz = 1024;
    stdin->base = stdin_buf;
    stdin->ptr = stdin_buf;
    stdin->cnt = 0;

    stdout = malloc(sizeof(FILE));
    stdout->file = STDOUT_FILENO;
    stdout->flag = _IOWRT;
    stdout->bufsiz = 1024;
    stdout->base = stdout_buf;
    stdout->ptr = stdout_buf;
    stdout->cnt = 0;

    stderr = malloc(sizeof(FILE));
    stderr->file = STDERR_FILENO;
    stderr->flag = _IOWRT;
    stderr->bufsiz = 1024;
    stderr->base = stderr_buf;
    stderr->ptr = stderr_buf;
    stderr->cnt = 0;
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

int _getchar(void) {
    int read_ret = read(STDIN_FILENO, stdin->ptr, 1);
    if (read_ret != 1) {
        return EOF;
    }

    stdin->ptr += read_ret;

    return stdin->base[stdin->cnt];
}

int getchar(void) {
    unsigned char ret = _getchar();
    if (ret <= 0) {
        return -1;
    }

    return ret;
}

static int _flush(void) {
    if (stdout->cnt == 0) {
        return 0;
    }

    int ret = write(STDOUT_FILENO, stdout->ptr, stdout->cnt);
    stdout->cnt = 0;
    return ret;
}

static int _putchar(char c) {
    if (stdout->cnt == 1024) {
        _flush();
    }

    stdout->ptr[stdout->cnt++] = c;
    return 0;
}

int putchar(char c) {
    _putchar(c);
    return _flush();
}

static int _puts(const char *str) {
    while (*str) {
        if (_putchar(*str++) == EOF) {
            return EOF;
        }
    }

    return 0;
}

int puts(const char *str) {
    if (_puts(str) == EOF) {
        return EOF;
    }
    _putchar('\n');

    return _flush();
}

char *gets(char *str) {
    int pos = read(0, str, 1024);
    if (pos == 0) {
        return NULL;
    }
    str[pos] = 0;

    return str;
}

char *fgets(char *str, int n, FILE *stream) {
    int pos = read(stream->file, str, n);
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
    char buf[1024] = "";

    int    modifier         = 0;
    char   curr_mod         = 0;
    bool   padding          = false;
    char   padding_char     = 0;
    int    width            = 0;
    size_t buf_len          = 0;
    bool   precision_toggle = false;
    int    precision        = 0;
    bool   left_align       = false;

    int ret = 0;

    while (*format) {
        char *old_str = str;

        if (modifier || padding || curr_mod || *format == '%') {
            format++;

            switch (*format) {
                case '%':
                    *str = '%';
                    str++;
                    break;

                case '-':
                    curr_mod = *format;
                    left_align = true;
                    continue;

                case ' ':
                    curr_mod = *format;
                    padding = true;
                    padding_char = ' ';
                    continue;

                case '0':
                    if (!padding && !curr_mod) {
                        curr_mod = *format;
                        padding = true;
                        padding_char = '0';
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
                    if (precision_toggle) {
                        precision *= 10;
                        precision += *format - '0';
                    } else {
                        if (!curr_mod) {
                            curr_mod = ' ';
                        }

                        width *= 10;
                        width += *format - '0';
                    }

                    continue;

                case '.':
                    curr_mod = *format;
                    precision_toggle = true;
                    continue;

                case '*':
                    curr_mod = *format;
                    continue;

                case 'l':
                    modifier++;
                    continue;

                case 'h':
                    modifier--;
                    continue;

                case 'c':
                    *str = (char)va_arg(args, int);
                    str++;
                    break;

                case 's':
                    if (precision_toggle) {
                        if (curr_mod == '*') {
                            buf_len = va_arg(args, int);
                        } else {
                            buf_len = precision;
                        }

                        strncpy(buf, va_arg(args, char *), buf_len);
                        for (int i = 0; i < buf_len; i++) {
                            if (buf[i] == 0) {
                                buf[i] = ' ';
                            }
                        }
                    } else {
                        strcpy(buf, va_arg(args, char *));
                        buf_len = strlen(buf);
                    }

                    if (width > buf_len) {
                        if (left_align) {
                            for (int i = buf_len; i < width; i++) {
                                buf[i] = ' ';
                            }
                            buf[width] = 0;
                            buf_len = width;
                        } else {
                            memmove(buf + width - buf_len, buf, buf_len + 1);
                            memset(buf, ' ', width - buf_len);
                        }
                    }

                    str = strncat(str, buf, buf_len);
                    str += buf_len;
                    break;

                case 'u':
                    if (modifier == 1) {
                        lutoa(va_arg(args, unsigned long int), buf, 10);
                    } else if (modifier == 0) {
                        utoa(va_arg(args, unsigned int), buf, 10);
                    } else if (modifier == -1) {
                        hutoa((unsigned short int)va_arg(args, unsigned int), buf, 10);
                    }

                    buf_len = strlen(buf);
                    if (padding && width > buf_len) {
                        memmove(buf + width - buf_len, buf, buf_len + 1);
                        memset(buf, padding_char, width - buf_len);
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
                    if (padding && width > buf_len) {
                        memmove(buf + width - buf_len, buf, buf_len + 1);
                        memset(buf, padding_char, width - buf_len);
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
                    if (padding && width > buf_len) {
                        memmove(buf + width - buf_len, buf, buf_len + 1);
                        memset(buf, padding_char, width - buf_len);
                    }

                    str = strcat(str, buf);
                    str += strlen(buf);
                    break;

                case 'b':
                    if (modifier == 1) {
                        lutoa(va_arg(args, unsigned long int), buf, 2);
                    } else if (modifier == 0) {
                        utoa(va_arg(args, unsigned int), buf, 2);
                    } else if (modifier == -1) {
                        hutoa((unsigned short int)va_arg(args, unsigned int), buf, 2);
                    }

                    buf_len = strlen(buf);
                    if (padding && width > buf_len) {
                        memmove(buf + width - buf_len, buf, buf_len + 1);
                        memset(buf, padding_char, width - buf_len);
                    }

                    str = strcat(str, buf);
                    str += strlen(buf);
                    break;

                case 'p':
                    modifier = 1;
                    padding = true;
                    width = 8;
                    padding_char = '0';
                case 'x':
                    if (modifier == 1) {
                        lutoa(va_arg(args, unsigned long int), buf, 16);
                    } else if (modifier == 0) {
                        utoa(va_arg(args, unsigned int), buf, 16);
                    } else if (modifier == -1) {
                        hutoa((unsigned short int)va_arg(args, unsigned int), buf, 16);
                    }

                    buf_len = strlen(buf);
                    if (padding && width > buf_len) {
                        memmove(buf + width - buf_len, buf, buf_len + 1);
                        memset(buf, padding_char, width - buf_len);
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
                    if (padding && width > buf_len) {
                        memmove(buf + width - buf_len, buf, buf_len + 1);
                        memset(buf, padding_char, width - buf_len);
                    }

                    str = strcat(str, strupr(buf));
                    str += strlen(buf);
                    break;

                case 'f':
                    if (modifier == 1) {
                        lftoa(va_arg(args, double), buf, precision_toggle ? precision : 6);
                    } else if (modifier == 0) {
                        ftoa((float)va_arg(args, double), buf, precision_toggle ? precision : 6);
                    }

                    buf_len = strlen(buf);
                    if (padding && width > buf_len) {
                        memmove(buf + width - buf_len, buf, buf_len + 1);
                        memset(buf, padding_char, width - buf_len);
                    }

                    str = strcat(str, buf);
                    str += strlen(buf);

                    break;
            }

            curr_mod         = 0;
            modifier         = 0;
            padding          = false;
            width            = 0;
            precision_toggle = false;
            precision        = 0;
        } else {
            *str = *format;
            str++;
        }

        ret += str - old_str;

        format++;
    }

    return ret;
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
