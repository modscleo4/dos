#include "vfs.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../bits.h"
#include "../debug.h"
#include "../rootfs.h"
#include "../cpu/panic.h"

mount_t *vfs_root = NULL;

static void vfs_describe_file(mount_t *mount, file_t *f, int level, bool recursive) {
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

    printf(" 1 %04ld\t%04ld", f->st.st_uid, f->st.st_gid);

    printf("\t%lu", f->st.st_size);

    printf("\t%lu", f->st.st_ctime);

    printf("\t%s", f->name);

    printf("\n");

    if (recursive && ISSET_BIT_INT(f->st.st_mode, S_IFDIR)) {
        dbgprint("Reading directory %s\n", f->name);
        size_t count = f->st.st_size / sizeof(file_t);
        for (int i = 0; ; i++) {
            file_t *out_f = (file_t *)calloc(1, sizeof(file_t));
            int readdir_ret = 0;
            if ((readdir_ret = mount->fs->readdir(mount->driver, mount->fs, &f->st, i, out_f->name, &out_f->st))) {
                dbgprint("Failed to readdir: %d\n", readdir_ret);
                break;
            }

            vfs_describe_file(mount, out_f, level + 1, (!strcmp(out_f->name, ".") || !strcmp(out_f->name, "..")) ? false : recursive);

            free(out_f->st.st_private);
            free(out_f);
        }
    }
}

void vfs_init(void) {
    dbgprint("Initializing VFS\n");
    vfs_root = (mount_t *)calloc(1, sizeof(mount_t));
    if (!vfs_root) {
        panic("Failed to allocate VFS root");
    }

    vfs_root->driver = &rootfs_io;
    vfs_root->fs = &rootfs;
    vfs_root->root.name[0] = '/';
    int stat_ret = 0;
    if ((stat_ret = rootfs.stat(&rootfs_io, &rootfs, "/", &vfs_root->root.st))) {
        free(vfs_root);
        vfs_root = NULL;
        panic("Failed to initialize VFS root: %d", stat_ret);
    }

    dbgprint("VFS root initialized\n");
    dbgprint("Reading root directory\n");
    vfs_describe_file(vfs_root, &vfs_root->root, 0, true);
}

bool vfs_mount(filesystem *fs, const char *path) {
    return false;
}

bool vfs_unmount(const char *path) {
    return false;
}

bool vfs_open(const char *path, file_t *file) {
    return false;
}

bool vfs_close(file_t *file) {
    return false;
}
