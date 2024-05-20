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
    file_t rootdir;
    ino_t ino;
    struct mount_t *parent;
} mount_t;

extern mount_t *root_mount;

void vfs_init(void);

int vfs_mount(iodriver *driver, filesystem *fs, const char *path);

int vfs_unmount(const char *path);

int vfs_open(const char *path, file_t *file);

int vfs_close(file_t *file);

int vfs_read(file_t *file, void *buf, size_t count, size_t offset);

int vfs_write(file_t *file, void *buf, size_t count, size_t offset);

int vfs_stat(const char *path, mount_t *cwd_mount, struct stat *cwd_st, mount_t *out_mount, struct stat *out_st);

void *vfs_load_file(mount_t *mount, struct stat *st);

void vfs_describe_file(mount_t *mount, file_t *f, int level, bool recursive);

#endif // VFS_H
