#include "dev.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../../drivers/fs/devfs.h"
#include "../../drivers/io/ramdisk.h"
#include "../../drivers/filesystem.h"

iodriver *devfs_driver;
filesystem *devfs;

void dev_init(void) {
    devfs_driver = (iodriver *)malloc(sizeof(iodriver));
    devfs = (filesystem *)malloc(sizeof(filesystem));

    ramdisk_init(devfs_driver, 1024 * 1024); // 1MiB
    devfs_init(devfs_driver, devfs);
}
