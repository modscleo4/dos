#ifndef KSTRING_H
#define KSTRING_H

#include <stddef.h>

typedef struct string_t {
    size_t len;
    char *str;
} string_t;

#endif
