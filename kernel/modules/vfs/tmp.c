#include "tmp.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../../drivers/fs/tmpfs.h"
#include "../../drivers/io/ramdisk.h"
#include "../../drivers/filesystem.h"

iodriver *tmpfs_driver;
filesystem *tmpfs;

void tmp_init(void) {
    tmpfs_driver = (iodriver *)malloc(sizeof(iodriver));
    tmpfs = (filesystem *)malloc(sizeof(filesystem));

    ramdisk_init(tmpfs_driver, 1024 * 1024); // 1MiB
    tmpfs_init(tmpfs_driver, tmpfs);
}
