#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>
#include <sys/stat.h>
#include "iodriver.h"

enum FileSystemType {
    FS_EMPTY = 0,
    FS_FAT12,
    FS_FAT16,
    FS_EXT2,
    FS_ISO9660,

    FS_TMPFS,
    FS_DEVFS,
    FS_PROCFS,
    FS_SYSFS,
    FS_PTYFS,
};

typedef struct filesystem {
    enum FileSystemType type;
    uint32_t start_lba;
    void *params;
    void *bitmap;
    char *name;
    struct stat *rootdir;
    bool case_sensitive;
    void (*init)(iodriver *driver, struct filesystem *fs);
    int (*stat)(iodriver *driver, struct filesystem *fs, const struct stat *st, const char *path, struct stat *out_st);
    int (*read)(iodriver *driver, struct filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);
    int (*write)(iodriver *driver, struct filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);
    int (*readdir)(iodriver *driver, struct filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st);
} filesystem;

#endif // FILESYSTEM_H
