#ifndef STRING_H
#define STRING_H

#include <ctype.h>
#include <stddef.h>

char *strupr(char *);

char *strlwr(char *);

int memcmp(const void *, const void *, size_t);

int strcmp(const char *, const char *);

size_t strlen(const char *);

void memcpy(void *, void *, size_t);

void *memset(void *, int, size_t);

char *strcpy(char *, const char *);

#endif //STRING_H
