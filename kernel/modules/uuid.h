#ifndef UUID_H
#define UUID_H

typedef struct uuid {
    unsigned int superlow;
    unsigned int low;
    unsigned int high;
    unsigned int superhigh;
} __attribute__((packed)) uuid;

const char *printuuid(uuid id);

#endif // UUID_H
