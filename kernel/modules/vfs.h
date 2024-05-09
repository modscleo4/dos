#ifndef VFS_H
#define VFS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "../drivers/filesystem.h"

typedef struct file_t {
    char name[256];
    struct stat st;
} file_t;

typedef struct mount_t {
    iodriver *driver;
    filesystem *fs;
    file_t root;
} mount_t;

extern mount_t *vfs_root;

void vfs_init(void);

bool vfs_mount(filesystem *fs, const char *path);

bool vfs_unmount(const char *path);

bool vfs_open(const char *path, file_t *file);

bool vfs_close(file_t *file);

#endif // VFS_H
