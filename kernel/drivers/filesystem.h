#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "iodriver.h"

enum FileSystem_Type {
    FS_EMPTY = 0,
    FS_FAT12,
    FS_FAT16,
};

typedef struct filesystem {
    unsigned char type;
    unsigned int start_lba;
    void (*init)(iodriver *, struct filesystem *);
    void *(*search_file)(iodriver *, struct filesystem *, const char *);
    void *(*load_file_at)(iodriver *, struct filesystem *, const void *, void *);
    void (*list_files)(iodriver *, struct filesystem *);
} filesystem;

filesystem rootfs;

#endif // FILESYSTEM_H
