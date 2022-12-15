#ifndef UUID_H
#define UUID_H

typedef struct uuid {
    unsigned int superlow;
    unsigned int low;
    unsigned int high;
    unsigned int superhigh;
} uuid;

const char *printuuid(uuid);

#endif // UUID_H
