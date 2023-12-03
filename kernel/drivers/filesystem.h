#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>
#include "iodriver.h"

enum FileSystemType {
    FS_EMPTY = 0,
    FS_FAT12,
    FS_FAT16,
    FS_EXT2,
    FS_ISO9660,
};

typedef struct filesystem {
    enum FileSystemType type;
    unsigned int start_lba;
    void *params;
    void (*init)(iodriver *driver, struct filesystem *fs);
    size_t (*get_file_size)(struct filesystem *fs, const void *_f);
    void *(*search_file)(iodriver *driver, struct filesystem *fs, const char *filename);
    void *(*load_file)(iodriver *driver, struct filesystem *fs, const void *_f);
    void *(*load_file_at)(iodriver *driver, struct filesystem *fs, const void *_f, void *addr);
    void (*list_files)(iodriver *driver, struct filesystem *fs);
} filesystem;

#endif // FILESYSTEM_H
