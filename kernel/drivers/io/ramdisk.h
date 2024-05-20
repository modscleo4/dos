#ifndef RAMDISK_H
#define RAMDISK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../iodriver.h"

void ramdisk_init(struct iodriver *driver, size_t size);

int ramdisk_read_sector(struct iodriver *driver, uint32_t lba, uint8_t *data, bool keepOn);

int ramdisk_write_sector(struct iodriver *driver, uint32_t lba, uint8_t *data, bool keepOn);

#endif // RAMDISK_H
