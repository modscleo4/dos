#ifndef FCNTL_H
#define FCNTL_H

enum OpenFlags {
    O_RDONLY = 0,
    O_WRONLY = 1,
    O_RDWR = 2,

    O_APPEND = 1024,
    O_ASYNC = 8192,
    O_CLOEXEC = 524288,
    O_CREAT = 64,
    O_DIRECT = 16384,
    O_DIRECTORY = 65536,
    O_DSYNC = 4096,
    O_EXCL = 128,
    O_LARGEFILE = 0,
    O_NOATIME = 262144,
    O_NOCTTY = 256,
    O_NOFOLLOW = 131072,
    O_NONBLOCK = 2048,
    O_NDELAY = 2048,
    O_PATH = 2097152,
    O_SYNC = 1052672,
    O_TMPFILE = 4194304,
    O_TRUNC = 512
};

#endif // FCNTL_H
