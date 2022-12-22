#ifndef IODRIVER_H
#define IODRIVER_H

#include <stdbool.h>

typedef struct iodriver {
    unsigned int device;
    unsigned char *io_buffer;
    unsigned int sector_size;
    void *partitions;
    int (*reset)(struct iodriver *driver);
    void (*start)(struct iodriver *driver);
    void (*stop)(struct iodriver *driver);
    int (*read_sector)(struct iodriver *driver, unsigned long int lba, unsigned char *data, bool keepOn);
    int (*write_sector)(struct iodriver *driver, unsigned long int lba, unsigned char *data, bool keepOn);
} iodriver;

typedef enum {
    io_read = 0,
    io_write = 1
} io_operation;

#endif // IODRIVER_H
