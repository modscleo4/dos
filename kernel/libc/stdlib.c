#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include "../drivers/floppy.h"
#include "../kernel.h"

float atof(const char *str) {
    return 0.0F;
}

int atoi(const char *str) {
    return 0;
}

char *htoa(short int value, char *str, int base) {
    return ltoa((long int)value, str, base);
}

char *itoa(int value, char *str, int base) {
    return ltoa((long int)value, str, base);
}

char *ltoa(long int value, char *str, int base) {
    char *rc;
    char *ptr;
    char *low;

    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    rc = ptr = str;

    if (value < 0 && base == 10) {
        *ptr++ = '-';
    }

    low = ptr;

    do {
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while (value);

    *ptr-- = '\0';

    while (low < ptr) {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }

    return rc;
}

char *hutoa(unsigned short int value, char *str, int base) {
    return lutoa((unsigned long int)value, str, base);
}

char *utoa(unsigned int value, char *str, int base) {
    return lutoa((unsigned long int)value, str, base);
}

char *lutoa(unsigned long int value, char *str, int base) {
    char *rc;
    char *ptr;
    char *low;

    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    rc = ptr = str;

    low = ptr;

    do {
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while (value);

    *ptr-- = '\0';

    while (low < ptr) {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }

    return rc;
}

char *ftoa(float value, char *str, int precision) {
    if (isnanf(value)) {
        strcpy(str, "nan");
        return str;
    } else if (isinff(value)) {
        if (value < 0) {
            strcpy(str, "-inf");
        } else {
            strcpy(str, "inf");
        }

        return str;
    }

    return lftoa((double)value, str, precision);
}

char *lftoa(double value, char *str, int precision) {
    if (isnanl(value)) {
        strcpy(str, "nan");
        return str;
    } else if (isinfl(value)) {
        if (value < 0) {
            strcpy(str, "-inf");
        } else {
            strcpy(str, "inf");
        }

        return str;
    }

    ltoa((long int)value, str, 10);
    int i = strlen(str);
    if (precision > 0) {
        str[i] = '.';

        value -= (long int)value;
        value *= powl(10, precision);
        ltoa((long int)value, str + i + 1, 10);

        int j = strlen(str + i + 1);
        while (j < precision) {
            str[i + j + 1] = '0';
            str[i + j + 2] = 0;
            j++;
        }
    }

    return str;
}

long int atol(const char *str) {
    return 0;
}

double strtod(const char *str, char **endptr) {
    return 0.0F;
}

long int strtol(const char *str, char **endptr, int base) {
    return 0;
}

unsigned long int strtoul(const char *str, char **endptr, int base) {
    return 0;
}

void *calloc(size_t num, size_t size) {
    return NULL;
}

void free(void *ptr) {
    //
}

void *malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    return (void *)0x100000;
}

void *realloc(void *ptr, size_t size) {
    return NULL;
}

void abort(void) {
    //
}

int atexit(void (*func)(void)) {
    return 0;
}

void exit(int status) {
    //
}

char *getenv(const char *name) {
    return NULL;
}

int system(const char *command) {
    fat_entry f;
    if (fat_search_file(boot_drive, command, &f)) {
        dbgprint("Not found.\n");
        return -1;
    }

    void *addr = malloc(f.size);
    if (!addr) {
        dbgprint("Allocation failed\n");
        return -1;
    }

    if (!fat_load_file_at(boot_drive, &f, addr)) {
        dbgprint("Not found.\n");
        return -1;
    }

    dbgprint("%s loaded at address %x\n", command, addr);
    addr += 0x1000;

    set_kernel_stack(0x110000);

    switch_ring3(addr);

    return 0;
}

void *bsearch(const void *key, const void *base, size_t num, size_t size, int (*compar)(const void *, const void *)) {
    return NULL;
}

void qsort(void *base, size_t num, size_t size, int (*compar)(const void *, const void *)) {
    //
}

div_t div(int numer, int denom) {
    return (div_t){
        0,
        0
    };
}

ldiv_t ldiv(long int numer, long int denom) {
    return (ldiv_t){
        0,
        0};
}

int abs(int x) {
    return (int)labs((long int)x);
}

long int labs(long int x) {
    return x < 0 ? -x : x;
}

int rand(void) {
    return 0;
}

void srand(unsigned int seed) {
    //
}

int mblen(const char *pmb, size_t max) {
    return 0;
}

size_t mbstowcs(wchar_t *dest, const char *src, size_t max) {
    return 0;
}

int mbtowc(wchar_t *pwc, const char *pmb, size_t max) {
    return 0;
}

size_t wcstombs(char *dest, const wchar_t *src, size_t max) {
    return 0;
}

int wctomb(char *pmb, wchar_t wc) {
    return 0;
}
