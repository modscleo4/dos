#include "procfs.h"

#include <errno.h>
#include "../../debug.h"

void procfs_init(iodriver *driver, filesystem *fs) {
    fs->type = FS_PROCFS;
    fs->name = "procfs";
    fs->start_lba = 0;
    fs->params = NULL;
    fs->init = procfs_init;
    fs->stat = procfs_stat;
    fs->read = procfs_read;
    fs->write = procfs_write;
    fs->readdir = procfs_readdir;
}

int procfs_stat(iodriver *driver, filesystem *fs, const struct stat *st, const char *path, struct stat *out_st) {
    return 0;
}

int procfs_read(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset) {
    return 0;
}

int procfs_write(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset) {
    // Read-only filesystem
    return -EROFS;
}

int procfs_readdir(iodriver *driver, filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st) {
    dbgprint("procfs_readdir\n");
    return -ENOENT;
}
