#include "uuid.h"

#include <stdio.h>

const char *printuuid(uuid id) {
    static char buf[37];
    sprintf(
        buf,
        "%08x-%04x-%04x-%04x-%04x%08x",
        id.superlow,
        id.low >> 16,
        id.low & 0xffff,
        id.high >> 16,
        id.high & 0xffff,
        id.superhigh
    );

    return buf;
}
