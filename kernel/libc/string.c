#include <string.h>

#include <ctype.h>

void memcpy(void *dest, const void *source, size_t n) {
    unsigned char *c_src = (unsigned char *)source;
    unsigned char *c_dest = (unsigned char *)dest;

    for (int i = 0; i < n; i++) {
        c_dest[i] = c_src[i];
    }
}

void strcpy(char *destination, const char *source) {
    strncpy(destination, source, strlen(source));
}

void strncpy(char *destination, const char *source, size_t n) {
    memcpy(destination, source, n);

    destination[n] = 0;
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
    memcpy(destination + strlen(destination), source, n);

    destination[n] = 0;
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
    unsigned char *c_ptr1 = (unsigned char *)ptr1;
    unsigned char *c_ptr2 = (unsigned char *)ptr2;

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
    return memcmp(str1, str2, strlen(str1));
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
    unsigned char *c_ptr = ptr;

    while (num > 0) {
        *c_ptr = (unsigned char)value;
        c_ptr++;
        num--;
    }

    return ptr;
}
