#ifndef DIRENT_H
#define DIRENT_H

#include <sys/types.h>

#define NAME_MAX 256

typedef struct DIR {
    int fd;
} DIR;

struct dirent {
    ino_t d_ino;             /* Inode number */
    off_t d_off;             /* Not an offset; see below */
    unsigned short d_reclen; /* Length of this record */
    unsigned char d_type;    /* Type of file; not supported by all filesystem types */
    char d_name[NAME_MAX];   /* Null-terminated filename */
};

#endif // DIRENT_H
