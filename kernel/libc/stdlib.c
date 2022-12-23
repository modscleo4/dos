#include <stdlib.h>

#include "../debug.h"
#include "../kernel.h"
#include "../ring3.h"
#include "../rootfs.h"
#include "../cpu/gdt.h"
#include "../modules/elf.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

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

    if (value < 0) {
        if (base == 10) {
            *ptr++ = '-';
        } else {
            value = -value;
        }
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

uint32_t __curr_malloc_addr = 0x1000000;

void *calloc(size_t num, size_t size) {
    void *addr = malloc(num * size);
    if (addr) {
        memset(addr, 0, num * size);
    }

    return addr;
}

void free(void *ptr) {
    //
}

void *malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    void *addr = (void *)__curr_malloc_addr;

    __curr_malloc_addr += size;

    return addr;
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
    void *f = rootfs.search_file(&rootfs_io, &rootfs, command);
    if (!f) {
        dbgprint("Not found.\n");
        return -1;
    }

    void *addr;
    if (!(addr = rootfs.load_file(&rootfs_io, &rootfs, f))) {
        dbgprint("Could not allocate or load file.\n");
        return -1;
    }

    dbgprint("%s loaded at address 0x%x\n", command, addr);

    elf_header_ident *_header = (elf_header_ident *) addr;
    if (memcmp(_header->magic, elf_magic, 4)) {
        dbgprint("Not an ELF file\n");
        return -1;
    }

    if (_header->version == ELF_ARCH_X86) {
        elf32_header *exec_header = (elf32_header *) addr;

        dbgprint("x86 ELF file\n");
        dbgprint("Entry point: %x\n", exec_header->entry);

        elf32_section_header *section_text = elf32_find_section(exec_header, ".text");
        if (!section_text) {
            dbgprint("No .text section found\n");
            return -1;
        }

        switch_ring3((uint32_t)(addr + section_text->offset + exec_header->entry), (uint32_t)(addr + 0x10000));
    } else if (_header->version == ELF_ARCH_X86_64) {
        elf64_header *exec_header = (elf64_header *) addr;

        dbgprint("x86_64 ELF file\n");
        dbgprint("Entry point: %x\n", exec_header->entry);

        elf64_section_header *section_text = elf64_find_section(exec_header, ".text");
        if (!section_text) {
            dbgprint("No .text section found\n");
            return -1;
        }

        switch_ring3((uint32_t)(addr + section_text->offset + exec_header->entry), (uint32_t)(addr + 0x10000));
    } else {
        dbgprint("Unsupported architecture: %x\n", _header->version);
        return -1;
    }

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
