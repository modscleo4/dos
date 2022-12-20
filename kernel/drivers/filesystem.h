#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "iodriver.h"

enum FileSystem_Type {
    FS_EMPTY = 0,
    FS_FAT12,
    FS_FAT16,
    FS_EXT2,
};

typedef struct filesystem {
    unsigned char type;
    unsigned int start_lba;
    void (*init)(iodriver *driver, struct filesystem *fs);
    unsigned long int (*get_file_size)(struct filesystem *fs, const void *_f);
    void *(*search_file)(iodriver *driver, struct filesystem *fs, const char *filename);
    void *(*load_file)(iodriver *driver, struct filesystem *fs, const void *_f);
    void *(*load_file_at)(iodriver *driver, struct filesystem *fs, const void *_f, void *addr);
    void (*list_files)(iodriver *driver, struct filesystem *fs);
} filesystem;

filesystem rootfs;

#endif // FILESYSTEM_H
