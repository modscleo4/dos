#ifndef DEVFS_H
#define DEVFS_H

#include <stddef.h>
#include "../filesystem.h"

typedef struct devfs_entry {
    char *name;
    struct stat *st;
} devfs_entry;

void devfs_init(iodriver *driver, filesystem *fs);

int devfs_stat(iodriver *driver, filesystem *fs, const struct stat *st, const char *path, struct stat *out_st);

int devfs_read(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);

int devfs_write(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);

int devfs_readdir(iodriver *driver, filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st);

#endif // DEVFS_H
