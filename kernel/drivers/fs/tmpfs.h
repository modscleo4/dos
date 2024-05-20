#ifndef TMPFS_H
#define TMPFS_H

#include <stddef.h>
#include "../filesystem.h"

typedef struct tmpfs_params {
    size_t size;
    void *data;
} tmpfs_params;

typedef struct tmpfs_directory_entry {
    char name[256];
    struct stat *st;
} tmpfs_directory_entry;

void tmpfs_init(iodriver *driver, filesystem *fs);

int tmpfs_stat(iodriver *driver, filesystem *fs, const struct stat *st, const char *path, struct stat *out_st);

int tmpfs_read(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);

int tmpfs_write(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);

int tmpfs_readdir(iodriver *driver, filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st);

#endif // TMPFS_H
