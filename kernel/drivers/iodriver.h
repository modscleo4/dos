#ifndef IODRIVER_H
#define IODRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct iodriver {
    int16_t device;
    uint8_t *io_buffer;
    uint16_t sector_size;
    size_t size;
    void *partitions;
    int (*reset)(struct iodriver *driver);
    void (*start)(struct iodriver *driver);
    void (*stop)(struct iodriver *driver);
    int (*read_sector)(struct iodriver *driver, uint32_t lba, uint8_t *data, bool keepOn);
    int (*write_sector)(struct iodriver *driver, uint32_t lba, uint8_t *data, bool keepOn);
} iodriver;

typedef enum IOOperation {
    IO_READ = 0,
    IO_WRITE = 1
} IOOperation;

#endif // IODRIVER_H
