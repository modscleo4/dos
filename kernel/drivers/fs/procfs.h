#ifndef DEVFS_H
#define DEVFS_H

#include <stddef.h>
#include "../filesystem.h"

void procfs_init(iodriver *driver, filesystem *fs);

int procfs_stat(iodriver *driver, filesystem *fs, const struct stat *st, const char *path, struct stat *out_st);

int procfs_read(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);

int procfs_write(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);

int procfs_readdir(iodriver *driver, filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st);

#endif // DEVFS_H
