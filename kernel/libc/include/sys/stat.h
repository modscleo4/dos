#ifndef STAT_H
#define STAT_H

#include <sys/types.h>

struct stat {
    dev_t st_dev;         /* ID of device containing file */
    ino_t st_ino;         /* inode number */
    mode_t st_mode;       /* protection */
    nlink_t st_nlink;     /* number of hard links */
    uid_t st_uid;         /* user ID of owner */
    gid_t st_gid;         /* group ID of owner */
    dev_t st_rdev;        /* device ID (if special file) */
    off_t st_size;        /* total size, in bytes */
    blksize_t st_blksize; /* blocksize for file system I/O */
    blkcnt_t st_blocks;   /* number of 512B blocks allocated */
    time_t st_atime;      /* time of last access */
    time_t st_mtime;      /* time of last modification */
    time_t st_ctime;      /* time of last status change */
    void *st_private;     /* private data used by the implementation */
};

enum FileTypes {
    S_IFMT = 0xF000,   /* Type of file descriptor */
    S_IFBLK = 0x6000,  /* block device */
    S_IFCHR = 0x2000,  /* character device */
    S_IFIFO = 0x1000,  /* FIFO (named pipe) */
    S_IFREG = 0x8000,  /* regular */
    S_IFDIR = 0x4000,  /* directory */
    S_IFLNK = 0xA000,  /* symbolic link */
    S_IFSOCK = 0xC000, /* socket */
};

enum FileModes {
    S_IRWXU = 00700, /* Read, write, execute/search by owner */
    S_IRUSR = 00400, /* Read permission, owner */
    S_IWUSR = 00200, /* Write permission, owner */
    S_IXUSR = 00100, /* Execute/search permission, owner */
    S_IRWXG = 00070, /* Read, write, execute/search by group */
    S_IRGRP = 00040, /* Read permission, group */
    S_IWGRP = 00020, /* Write permission, group */
    S_IXGRP = 00010, /* Execute/search permission, group */
    S_IRWXO = 00007, /* Read, write, execute/search by others */
    S_IROTH = 00004, /* Read permission, others */
    S_IWOTH = 00002, /* Write permission, others */
    S_IXOTH = 00001, /* Execute/search permission, others */
    S_ISVTX = 01000, /* Sticky bit */
    S_ISGID = 02000, /* Set-group-ID on execution */
    S_ISUID = 04000, /* Set-user-ID on execution */
};

#endif // STAT_H
