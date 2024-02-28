#ifndef UUID_H
#define UUID_H

#include <stdint.h>

typedef struct uuid {
    uint32_t superlow;
    uint32_t low;
    uint32_t high;
    uint32_t superhigh;
} __attribute__((packed)) uuid;

const char *printuuid(uuid id);

#endif // UUID_H
