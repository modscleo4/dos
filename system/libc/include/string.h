#ifndef STRING_H
#define STRING_H

#include <ctype.h>
#include <stddef.h>

void memcpy(void *, const void *, size_t);

void strcpy(char *, const char *);

void strncpy(char *, const char *, size_t n);

char *strupr(char *);

char *strlwr(char *);

int memcmp(const void *, const void *, size_t);

int strcmp(const char *, const char *);

size_t strlen(const char *);

void *memset(void *, int, size_t);

#endif //STRING_H
