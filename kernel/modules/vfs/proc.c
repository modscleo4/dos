#include "proc.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../../drivers/fs/procfs.h"
#include "../../drivers/io/ramdisk.h"
#include "../../drivers/filesystem.h"

iodriver *procfs_driver;
filesystem *procfs;

void proc_init(void) {
    procfs_driver = (iodriver *)malloc(sizeof(iodriver));
    procfs = (filesystem *)malloc(sizeof(filesystem));

    ramdisk_init(procfs_driver, 1024 * 1024); // 1MiB
    procfs_init(procfs_driver, procfs);
}
