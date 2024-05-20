#include "vfs.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"
#include "../bits.h"
#include "../debug.h"
#include "../rootfs.h"
#include "../cpu/panic.h"

mount_t *root_mount = NULL;
static struct {
    mount_t *m;
    size_t count;
    size_t capacity;
} mounts;

void vfs_init(void) {
    dbgprint("Initializing VFS\n");
    root_mount = (mount_t *)calloc(1, sizeof(mount_t));
    if (!root_mount) {
        panic("Failed to allocate VFS root");
    }

    root_mount->driver = &rootfs_io;
    root_mount->fs = &rootfs;
    root_mount->rootdir.name[0] = '/';
    memcpy(&root_mount->rootdir.st, root_mount->fs->rootdir, sizeof(struct stat));
    root_mount->ino = root_mount->rootdir.st.st_ino;

    dbgprint("VFS root initialized\n");

    mounts.count = 0;
    mounts.capacity = 10;
    mounts.m = (mount_t *)calloc(mounts.capacity, sizeof(mount_t));
    if (!mounts.m) {
        panic("Failed to allocate mounts");
    }
}

int vfs_mount(iodriver *driver, filesystem *fs, const char *path) {
    if (!root_mount) {
        dbgprint("VFS root not initialized\n");
        return -ENODEV;
    }

    if (!driver || !fs || !path) {
        dbgprint("Invalid arguments\n");
        return -EINVAL;
    }

    if (mounts.count == mounts.capacity) {
        dbgprint("No more mounts available\n");
        return -ENOSPC;
    }

    if (strlen(path) == 0 || path[0] != '/') {
        dbgprint("Invalid path\n");
        return -EINVAL;
    }

    struct stat st;
    if (vfs_stat(path, root_mount, &root_mount->rootdir.st, NULL, &st) < 0) {
        dbgprint("No such file or directory: %s\n", path);
        return -ENOENT;
    }

    for (int i = 0; i < mounts.count; i++) {
        if (mounts.m[i].ino == st.st_ino) {
            dbgprint("Filesystem already mounted: %s\n", path);
            dbgprint("Inode: %ld\n", st.st_ino);
            return -EEXIST;
        }
    }

    mount_t *mount = &mounts.m[mounts.count++];

    mount->driver = driver;
    mount->fs = fs;
    mount->ino = st.st_ino;
    memcpy(&mount->rootdir.st, fs->rootdir, sizeof(struct stat));

    strncpy(mount->rootdir.name, path, sizeof(mount->rootdir.name));

    dbgprint("Mounted %s on %s (ino: %ld)\n", fs->name, path, st.st_ino);

    return 0;
}

int vfs_unmount(const char *path) {
    return -EIO;
}

int vfs_open(const char *path, file_t *file) {
    return -EIO;
}

int vfs_close(file_t *file) {
    return -EIO;
}

int vfs_read(file_t *file, void *buf, size_t count, size_t offset) {
    return -EIO;
}

int vfs_write(file_t *file, void *buf, size_t count, size_t offset) {
    return -EROFS;
}

int vfs_stat(const char *path, mount_t *cwd_mount, struct stat *st, mount_t *out_mount, struct stat *out_st) {
    if (!root_mount) {
        dbgprint("VFS root not initialized\n");
        return -ENODEV;
    }

    if (!path) {
        dbgprint("Invalid arguments\n");
        return -EINVAL;
    }

    if (strlen(path) == 0) {
        dbgprint("Invalid path\n");
        return -EINVAL;
    }

    if (!strcmp(path, "/")) {
        if (out_st) {
            memcpy(out_st, &root_mount->rootdir.st, sizeof(struct stat));
        }

        return 0;
    }

    char *fn = strdup(path);
    char *filename = strtok(fn[0] == '/' ? fn + 1 : fn, "/");
    mount_t *mount = root_mount;
    struct stat dir = mount->rootdir.st;
    file_t *f = (file_t *)calloc(1, sizeof(file_t));
    if (!f) {
        dbgprint("Failed to allocate file\n");
        free(fn);
        return -ENOMEM;
    }

    strcpy(f->name, "/");
    f->st = mount->rootdir.st;

    while (filename && *filename) {
        int readdir_ret = 0;
        for (int i = 0;; i++) {
            if ((readdir_ret = mount->fs->readdir(mount->driver, mount->fs, &dir, i, f->name, &f->st)) < 0) {
                if (readdir_ret == -ENOENT) {
                    break;
                }

                dbgprint("Failed to readdir: %d\n", readdir_ret);
                free(f);
                free(fn);
                return readdir_ret;
            }

            if (
                mount->fs->case_sensitive && strcmp(f->name, filename) == 0
                || !mount->fs->case_sensitive && stricmp(f->name, filename) == 0
            ) {
                if (ISSET_BIT_INT(f->st.st_mode, S_IFDIR)) {
                    dir = f->st;

                    // Check if the directory is mounted
                    for (int i = 0; i < mounts.count; i++) {
                        if (mounts.m[i].ino == dir.st_ino) {
                            mount = &mounts.m[i];
                            dir = mount->rootdir.st;
                            break;
                        }
                    }
                }

                goto next;
            }
        }

        dbgprint("Failed to locate %s\n", filename);
        free(f);
        free(fn);
        return -ENOENT;

    next:
        // free(f->st.st_private);
        filename = strtok(NULL, "/");
    }

    if (out_mount) {
        memcpy(out_mount, mount, sizeof(mount_t));
    }

    if (out_st) {
        memcpy(out_st, &f->st, sizeof(struct stat));
    }

    free(f);
    free(fn);

    return 0;
}

void *vfs_load_file(mount_t *mount, struct stat *st) {
    void *addr = malloc_align(st->st_size, BITMAP_PAGE_SIZE);
    int read_ret = 0;
    if ((read_ret = mount->fs->read(mount->driver, mount->fs, st, addr, st->st_size, 0)) < 0) {
        dbgprint("Failed to read file: %d\n", read_ret);
        free(addr);
        return NULL;
    }

    return addr;
}

void vfs_describe_file(mount_t *mount, file_t *f, int level, bool recursive) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    if (ISSET_BIT_INT(f->st.st_mode, S_IFIFO)) {
        printf("p");
    } else if (ISSET_BIT_INT(f->st.st_mode, S_IFCHR)) {
        printf("c");
    } else if (ISSET_BIT_INT(f->st.st_mode, S_IFDIR)) {
        printf("d");
    } else if (ISSET_BIT_INT(f->st.st_mode, S_IFBLK)) {
        printf("b");
    } else if (ISSET_BIT_INT(f->st.st_mode, S_IFREG)) {
        printf("-");
    } else if (ISSET_BIT_INT(f->st.st_mode, S_IFLNK)) {
        printf("l");
    } else if (ISSET_BIT_INT(f->st.st_mode, S_IFSOCK)) {
        printf("s");
    } else {
        printf("?");
    }

    printf("%s", ISSET_BIT_INT(f->st.st_mode, S_IRUSR) ? "r" : "-");
    printf("%s", ISSET_BIT_INT(f->st.st_mode, S_IWUSR) ? "w" : "-");
    printf("%s", ISSET_BIT_INT(f->st.st_mode, S_IXUSR) ? "x" : "-");

    printf("%s", ISSET_BIT_INT(f->st.st_mode, S_IRGRP) ? "r" : "-");
    printf("%s", ISSET_BIT_INT(f->st.st_mode, S_IWGRP) ? "w" : "-");
    printf("%s", ISSET_BIT_INT(f->st.st_mode, S_IXGRP) ? "x" : "-");

    printf("%s", ISSET_BIT_INT(f->st.st_mode, S_IROTH) ? "r" : "-");
    printf("%s", ISSET_BIT_INT(f->st.st_mode, S_IWOTH) ? "w" : "-");
    printf("%s", ISSET_BIT_INT(f->st.st_mode, S_IXOTH) ? "x" : "-");

    printf(" %ld %04ld\t%04ld", f->st.st_ino, f->st.st_uid, f->st.st_gid);

    printf("\t%lu", f->st.st_size);

    printf("\t%lu", f->st.st_ctime);

    printf("\t%s", f->name);

    printf("\n");

    if (recursive && ISSET_BIT_INT(f->st.st_mode, S_IFDIR)) {
        dbgprint("Reading directory %s\n", f->name);
        struct stat *dir = &f->st;
        size_t count = f->st.st_size / sizeof(file_t);
        file_t *out_f = (file_t *)calloc(1, sizeof(file_t));
        for (int i = 0;; i++) {
            mount_t *m = mount;
            // Check if the directory is mounted
            for (int i = 0; i < mounts.count; i++) {
                if (mounts.m[i].ino == f->st.st_ino) {
                    m = &mounts.m[i];
                    dir = &m->rootdir.st;
                    break;
                }
            }

            int readdir_ret = 0;
            if ((readdir_ret = m->fs->readdir(m->driver, m->fs, &f->st, i, out_f->name, &out_f->st)) < 0) {
                dbgprint("Failed to readdir: %d\n", readdir_ret);
                break;
            }

            vfs_describe_file(m, out_f, level + 1, (!strcmp(out_f->name, ".") || !strcmp(out_f->name, "..")) ? false : recursive);

            free(out_f->st.st_private);
        }

        free(out_f);
    }
}
