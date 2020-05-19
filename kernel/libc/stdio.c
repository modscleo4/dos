#include <stdio.h>
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
    return EOF;
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
