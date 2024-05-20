#ifndef FD_H
#define FD_H

#include "vfs.h"
#include "../drivers/filesystem.h"
#include "../drivers/iodriver.h"
#include "../drivers/tty.h"

typedef struct file_descriptor {
    bool used;
    enum FileTypes type;
    mount_t mount;
    struct stat st;
    char name[256];
    tty_t *tty;
    framebuffer_t *fb;
    uint32_t offset;
    uint32_t flags;
    struct {
        bool read;
        bool write;
    } access;
} file_descriptor;

#endif // FD_H
