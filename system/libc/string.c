#include <string.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

void *memcpy(void *restrict destination, const void *restrict source, size_t n) {
    size_t i = 0;
    void *ret = destination;

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
    memcpy(destination, source, len);
    destination[len] = 0;

    return destination;
}

char *strncpy(char *destination, const char *source, size_t n) {
    size_t len = strlen(source);
    memcpy(destination, source, len < n ? len : n);

    for (size_t i = len; i < n; i++) {
        destination[i] = 0;
    }

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

char *strcat(char *destination, const char *source) {
    return strncat(destination, source, strlen(source));
}

char *strncat(char *destination, const char *source, size_t n) {
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

const void *memchr(const void *ptr, int value, size_t num) {
    unsigned char *ptr_c = (unsigned char *)ptr;

    for (int i = 0; i < num; i++) {
        if (ptr_c[i] == (unsigned char)value) {
            return &ptr_c[i];
        }
    }

    return NULL;
}

const char *strchr(const char *str, int character) {
    return memchr(str, character, strlen(str) + 1);
}

size_t strcspn(const char *str1, const char *str2) {
    size_t i = 0;
    for (i = 0; str1[i]; i++) {
        int j = 0;
        for (j = 0; str2[j]; j++) {
            if (str1[i] == str2[j]) {
                break;
            }
        }

        if (str2[j]) {
            break;
        }
    }

    return i;
}

char *strpbrk(const char *str1, const char *str2) {
    size_t i = 0;
    for (i = 0; str1[i]; i++) {
        int j = 0;
        for (j = 0; str2[j]; j++) {
            if (str1[i] == str2[j]) {
                return (char *)&str1[i];
            }
        }
    }

    return NULL;
}

char *strrchr(const char *str, int character) {
    size_t i = 0;
    for (i = strlen(str) - 1; i >= 0; i--) {
        if (str[i] == character) {
            return (char *)&str[i];
        }
    }

    return NULL;
}

size_t strspn(const char *str1, const char *str2) {
    size_t i = 0;
    for (i = 0; str1[i]; i++) {
        int j = 0;
        for (j = 0; str2[j]; j++) {
            if (str1[i] == str2[j]) {
                break;
            }
        }

        if (!str2[j]) {
            break;
        }
    }

    return i;
}

char *strstr(const char *str1, const char *str2) {
    size_t i = 0;
    for (i = 0; str1[i]; i++) {
        if (str1[i] == str2[0]) {
            size_t j = 0;
            bool found = true;
            for (j = 0; str2[j]; j++) {
                if (str1[i + j] != str2[j]) {
                    found = false;
                    break;
                }
            }

            if (found) {
                return (char *)&str1[i];
            }
        }
    }

    return NULL;
}

char *strtok(char *str, const char *delimiters) {
    static char *last = NULL;

    if (!str) {
        str = last;
    }

    if (!str) {
        return NULL;
    }

    size_t i = 0;
    for (i = 0; str[i]; i++) {
        int j = 0;
        for (j = 0; delimiters[j]; j++) {
            if (str[i] == delimiters[j]) {
                break;
            }
        }

        if (delimiters[j]) {
            break;
        }
    }

    if (!str[i]) {
        last = NULL;
        return str;
    }

    str[i] = 0;
    last = &str[i + 1];

    return str;
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

    asm volatile(
        "rep stosb"
        : "=&c"(d0), "=&D"(d1)
        : "a"((uint8_t)value), "1"(ptr), "0"(num)
        : "memory"
    );

    return ptr;
}
