#include "ramdisk.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "../../modules/bitmap.h"

void ramdisk_init(struct iodriver *driver, size_t size) {
    driver->sector_size = BITMAP_PAGE_SIZE;
    driver->io_buffer = malloc(driver->sector_size);
    driver->size = size;
    driver->partitions = malloc(driver->size);
    driver->reset = NULL;
    driver->start = NULL;
    driver->stop = NULL;
    driver->read_sector = ramdisk_read_sector;
    driver->write_sector = ramdisk_write_sector;
}

int ramdisk_read_sector(struct iodriver *driver, uint32_t lba, uint8_t *data, bool keepOn) {
    if (lba * driver->sector_size >= driver->size) return -EIO;

    memcpy(data, (uint8_t *)driver->partitions + lba * driver->sector_size, driver->sector_size);
    return 0;
}

int ramdisk_write_sector(struct iodriver *driver, uint32_t lba, uint8_t *data, bool keepOn) {
    if (lba * driver->sector_size >= driver->size) return -EIO;

    memcpy((uint8_t *)driver->partitions + lba * driver->sector_size, data, driver->sector_size);
    return 0;
}
