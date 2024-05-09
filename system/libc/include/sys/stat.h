#ifndef STAT_H
#define STAT_H

#include <sys/types.h>

struct stat {
    dev_t st_dev;
    ino_t st_ino;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
    time_t st_atime;
    time_t st_mtime;
    time_t st_ctime;
};

enum FileTypes {
    S_IFMT = 0xF000,
    S_IFBLK = 0x6000,
    S_IFCHR = 0x2000,
    S_IFIFO = 0x1000,
    S_IFREG = 0x8000,
    S_IFDIR = 0x4000,
    S_IFLNK = 0xA000,
};

enum FileModes {
    S_IRWXU = 00700,
    S_IRUSR = 00400,
    S_IWUSR = 00200,
    S_IXUSR = 00100,
    S_IRWXG = 00070,
    S_IRGRP = 00040,
    S_IWGRP = 00020,
    S_IXGRP = 00010,
    S_IRWXO = 00007,
    S_IROTH = 00004,
    S_IWOTH = 00002,
    S_IXOTH = 00001,
    S_ISUID = 04000,
    S_ISGID = 02000,
    S_ISVTX = 01000,
};

#define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)

int chmod(const char* pathname, mode_t mode);
int fchmod(int fd, mode_t mode);
int fstat(int fd, struct stat* buf);
int lstat(const char* path, struct stat* buf);
int mkdir(const char* pathname, mode_t mode);
int mkfifo(const char* pathname, mode_t mode);
int mknod(const char* pathname, mode_t mode, dev_t dev);
int stat(const char* path, struct stat* buf);
mode_t umask(mode_t mask);

#endif // STAT_H
