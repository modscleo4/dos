#include <string.h>

#include <ctype.h>
#include <stdint.h>
#include "../bits.h"
#include "../cpu/cpuid.h"

void *memcpy(void *restrict destination, const void *restrict source, size_t n) {
    size_t i = 0;
    void *ret = destination;

    if (ISSET_BIT_INT(cpuinfo.edx, CPUID_FEAT_EDX_SSE)) {
        for (i = 0; i < n / 16; i++) {
            asm volatile (
                "movdqu (%0), %%xmm0;"
                "movdqu %%xmm0, (%1);"
                :
                : "r"(source), "r"(destination)
                : "memory"
            );

            source += 16;
            destination += 16;
        }

        n -= i * 16;
    }

    if (ISSET_BIT_INT(cpuinfo.edx, CPUID_FEAT_EDX_MMX)) {
        for (i = 0; i < n / 8; i++) {
            asm volatile (
                "movq (%0), %%mm0;"
                "movq %%mm0, (%1);"
                :
                : "r"(source), "r"(destination)
                : "memory"
            );

            source += 8;
            destination += 8;
        }

        n -= i * 8;
    }

    for (i = 0; i < n / 4; i++) {
        *(uint32_t *)destination = *(uint32_t *)source;

        source += 4;
        destination += 4;
    }

    n -= i * 4;

    for (i = 0; i < n / 2; i++) {
        *(uint16_t *)destination = *(uint16_t *)source;

        source += 2;
        destination += 2;
    }

    n -= i * 2;

    uint8_t *c_src = (uint8_t *)source;
    uint8_t *c_dest = (uint8_t *)destination;

    while (n--) {
        *c_dest++ = *c_src++;
    }

    return ret;
}

char *strcpy(char *destination, const char *source) {
    size_t len = strlen(source);
    return strncpy(destination, source, len);
}

char *strncpy(char *destination, const char *source, size_t n) {
    memcpy(destination, source, n);

    destination[n] = 0;

    return destination;
}

void *memmove(void *dest, const void *source, size_t n) {
    if (dest < source) {
        memcpy(dest, source, n);
    } else {
        unsigned char *c_dest = (unsigned char *)dest;
        unsigned char *c_src = (unsigned char *)source;

        for (int i = n - 1; i >= 0; i--) {
            c_dest[i] = c_src[i];
        }
    }

    return dest;
}

char* strcat(char *destination, const char *source) {
    return strncat(destination, source, strlen(source));
}

char* strncat(char *destination, const char *source, size_t n) {
    size_t dest_len = strlen(destination);
    strncpy(destination + dest_len, source, n);

    return destination;
}

char *strupr(char *str) {
    char *ptr = str;

    while ((*ptr = (char)toupper(*ptr))) {
        ptr++;
    }

    return str;
}

char *strlwr(char *str) {
    char *ptr = str;

    while ((*ptr = (char)tolower(*ptr))) {
        ptr++;
    }

    return str;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
    uint8_t *c_ptr1 = (uint8_t *)ptr1;
    uint8_t *c_ptr2 = (uint8_t *)ptr2;

    while (num-- > 0) {
        if (*c_ptr1 != *c_ptr2) {
            return *c_ptr1 - *c_ptr2;
        }

        c_ptr1++;
        c_ptr2++;
    }

    return 0;
}

int strcmp(const char *str1, const char *str2) {
    if (!*str1) {
        return *str2;
    }

    if (!*str2) {
        return -*str1;
    }

    size_t len_1 = strlen(str1);
    size_t len_2 = strlen(str2);

    return strncmp(str1, str2, len_1 < len_2 ? len_1 : len_2) || (len_1 < len_2 ? str1[len_1] - str2[len_1] : str1[len_2] - str2[len_2]);
}

int strncmp(const char *str1, const char *str2, size_t num) {
    if (!*str1) {
        return *str2;
    }

    if (!*str2) {
        return -*str1;
    }

    return memcmp(str1, str2, num);
}

void *memchr(void *ptr, int value, size_t num) {
    unsigned char *ptr_c = (unsigned char *)ptr;

    for (int i = 0; i < num; i++) {
        if (ptr_c[i] == (unsigned char)value) {
            return &ptr_c[i];
        }
    }

    return NULL;
}

char *strchr(char *str, int character) {
    return memchr(str, character, strlen(str) + 1);
}

size_t strlen(const char *str) {
    size_t l = 0;
    while (*str++) {
        l++;
    }

    return l;
}

void *memset(void *ptr, int value, size_t num) {
    int d0;
    int d1;

    asm volatile (
        "rep stosb"
        : "=&c" (d0), "=&D" (d1)
        : "a" ((uint8_t) value), "1" (ptr), "0" (num)
        : "memory"
    );

    return ptr;
}
