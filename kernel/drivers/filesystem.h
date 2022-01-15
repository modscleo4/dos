#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "iodriver.h"

typedef struct filesystem {
    unsigned char type;
    void (*init)(iodriver *);
    int (*search_file)(iodriver *, const char *, void *);
    void *(*load_file_at)(iodriver *, const void *, void *);
    void (*list_files)(iodriver *);
} filesystem;

filesystem fs;

#endif // FILESYSTEM_H
