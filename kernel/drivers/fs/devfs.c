#include "devfs.h"

#define DEBUG 1

#include <errno.h>
#include "../../debug.h"

void devfs_init(iodriver *driver, filesystem *fs) {
    fs->type = FS_DEVFS;
    fs->name = "devfs";
    fs->start_lba = 0;
    fs->params = NULL;
    fs->init = devfs_init;
    fs->stat = devfs_stat;
    fs->read = devfs_read;
    fs->write = devfs_write;
    fs->readdir = devfs_readdir;
}

int devfs_stat(iodriver *driver, filesystem *fs, const struct stat *st, const char *path, struct stat *out_st) {
    return 0;
}

int devfs_read(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset) {
    return 0;
}

int devfs_write(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset) {
    // Read-only filesystem
    return -EROFS;
}

int devfs_readdir(iodriver *driver, filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st) {
    dbgprint("devfs_readdir\n");
    return -ENOENT;
}
