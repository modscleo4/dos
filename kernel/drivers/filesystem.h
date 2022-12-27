#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>
#include "iodriver.h"

typedef struct filesystem {
    unsigned char type;
    unsigned int start_lba;
    void *params;
    void (*init)(iodriver *driver, struct filesystem *fs);
    size_t (*get_file_size)(struct filesystem *fs, const void *_f);
    void *(*search_file)(iodriver *driver, struct filesystem *fs, const char *filename);
    void *(*load_file)(iodriver *driver, struct filesystem *fs, const void *_f);
    void *(*load_file_at)(iodriver *driver, struct filesystem *fs, const void *_f, void *addr);
    void (*list_files)(iodriver *driver, struct filesystem *fs);
} filesystem;

enum FileSystemType {
    FS_EMPTY = 0,
    FS_FAT12,
    FS_FAT16,
    FS_EXT2,
};

#endif // FILESYSTEM_H
