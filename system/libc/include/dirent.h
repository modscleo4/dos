#ifndef DIRENT_H
#define DIRENT_H

#include <sys/types.h>

typedef struct DIR DIR;

#define NAME_MAX 256

struct dirent {
    ino_t d_ino;             /* Inode number */
    off_t d_off;             /* Not an offset; see below */
    unsigned short d_reclen; /* Length of this record */
    unsigned char d_type;    /* Type of file; not supported by all filesystem types */
    char d_name[NAME_MAX];   /* Null-terminated filename */
};

enum DirentType {
    DT_BLK = 0x6000,  /* block device */
    DT_CHR = 0x2000,  /* character device */
    DT_IFO = 0x1000,  /* FIFO (named pipe) */
    DT_REG = 0x8000,  /* regular */
    DT_DIR = 0x4000,  /* directory */
    DT_LNK = 0xA000,  /* symbolic link */
    DT_SOCK = 0xC000, /* socket */
    DT_UNKNOWN = 0,
};

int closedir(DIR *dirp);
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
void rewinddir(DIR *dirp);
void seekdir(DIR *dirp, long int loc);
long telldir(DIR *dirp);

#endif // DIRENT_H
