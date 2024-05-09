#ifndef FD_H
#define FD_H

#include "../drivers/filesystem.h"
#include "../drivers/iodriver.h"

typedef struct file_descriptor {
    bool used;
    enum FileTypes type;
    iodriver *io;
    filesystem *fs;
    struct stat st;
    char path[256];
    uint32_t tty;
    bool tty_canon;
    uint32_t offset;
    uint32_t flags;
    struct {
        bool read;
        bool write;
    } access;
} file_descriptor;

#endif // FD_H
