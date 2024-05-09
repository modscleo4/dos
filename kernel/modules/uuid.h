#ifndef UUID_H
#define UUID_H

#include <stdint.h>

#pragma pack(push, 1)
typedef struct uuid {
    uint32_t superlow;
    uint32_t low;
    uint32_t high;
    uint32_t superhigh;
} uuid;
#pragma pack(pop)

const char *printuuid(uuid id);

#endif // UUID_H
