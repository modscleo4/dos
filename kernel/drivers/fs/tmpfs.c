#include "tmpfs.h"

#include <errno.h>
#include "../../debug.h"

void tmpfs_init(iodriver *driver, filesystem *fs) {
    fs->type = FS_TMPFS;
    fs->start_lba = 0;
    fs->params = NULL;
    fs->init = tmpfs_init;
    fs->stat = tmpfs_stat;
    fs->read = tmpfs_read;
    fs->write = tmpfs_write;
    fs->readdir = tmpfs_readdir;
}

int tmpfs_stat(iodriver *driver, filesystem *fs, const struct stat *st, const char *path, struct stat *out_st) {
    return 0;
}

int tmpfs_read(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset) {
    return 0;
}

int tmpfs_write(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset) {
    // Read-only filesystem
    return -EROFS;
}

int tmpfs_readdir(iodriver *driver, filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st) {
    dbgprint("tmpfs_readdir\n");
    return -ENOENT;
}
