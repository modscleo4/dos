#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void *memcpy(void *destination, const void *source, size_t n);

char *strcpy(char *destination, const char *source);

char *strncpy(char *destination, const char *source, size_t n);

void *memmove(void *dest, const void *source, size_t n);

char *strcat(char *destination, const char *source);

char *strncat(char *destination, const char *source, size_t n);

char *strupr(char *str);

char *strlwr(char *str);

int memcmp(const void *ptr1, const void *ptr2, size_t num);

int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t num);

const void *memchr(const void *ptr, int value, size_t num);

const char *strchr(const char *str, int character);

size_t strcspn(const char *str1, const char *str2);

char *strpbrk(const char *str1, const char *str2);

char *strrchr(const char *str, int character);

size_t strspn(const char *str1, const char *str2);

char *strstr(const char *str1, const char *str2);

char *strtok(char *str, const char *delimiters);

size_t strlen(const char *str);

void *memset(void *ptr, int value, size_t num);

#endif //STRING_H
