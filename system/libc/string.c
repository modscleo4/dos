#include <string.h>

char *strupr(char *str) {
    char *ptr = str;

    while ((*ptr = (char)toupper(*ptr)) != '\0') {
        ptr++;
    }

    return str;
}

char *strlwr(char *str) {
    char *ptr = str;

    while ((*ptr = (char)tolower(*ptr)) != '\0') {
        ptr++;
    }

    return str;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
    unsigned char *c_ptr1 = (unsigned char *)ptr1;
    unsigned char *c_ptr2 = (unsigned char *)ptr2;

    while (num-- > 0) {
        if (*c_ptr1 != *c_ptr2) {
            return *c_ptr1 - *c_ptr2;
        }
    }

    return 0;
}

int strcmp(const char *str1, const char *str2) {
    while (str1++ && str2++) {
        if (*str1 != *str2) {
            return *str1 - *str2;
        }
    }

    return 0;
}

size_t strlen(const char *str) {
    size_t l = 0;
    while (*str) {
        l++;
        str++;
    }

    return l;
}

void memcpy(void *source, void *dest, size_t n) {
    unsigned char *c_src = (unsigned char *)source;
    unsigned char *c_dest = (unsigned char *)dest;

    int i;
    for (i = 0; i < n; i++) {
        c_dest[i] = c_src[i];
    }
}

void *memset(void *ptr, int value, size_t num) {
    unsigned char *c_ptr = ptr;

    while (num > 0) {
        *c_ptr = (unsigned char)value;
        c_ptr++;
        num--;
    }

    return ptr;
}

char *strcpy(char *destination, const char *source) {
    memcpy(source, destination, strlen(source));
}
