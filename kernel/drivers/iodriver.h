#ifndef IODRIVER_H
#define IODRIVER_H

#include <stdbool.h>

typedef struct iodriver {
    unsigned int device;
    unsigned char *io_buffer;
    int (*reset)(unsigned int);
    void (*start)(unsigned int);
    void (*stop)(unsigned int);
    int (*read_sector)(unsigned int, unsigned long int, unsigned char *, bool);
    int (*write_sector)(unsigned int, unsigned long int, unsigned char *, bool);
} iodriver;

typedef enum {
    io_read = 0,
    io_write = 1
} io_operation;

iodriver rootfs_io;

#endif // IODRIVER_H
