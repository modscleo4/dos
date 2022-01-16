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
    void (*init)(iodriver *);
    int (*search_file)(iodriver *, const char *, void *, unsigned char);
    void *(*load_file_at)(iodriver *, const void *, void *, unsigned char);
    void (*list_files)(iodriver *, unsigned char);
} filesystem;

filesystem fs;

#endif // FILESYSTEM_H
