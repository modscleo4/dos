#include <sys/stat.h>
#include <unistd.h>

int chmod(const char *pathname, mode_t mode) {
    return syscall(90, pathname, mode);
}

int fchmod(int fd, mode_t mode) {
    return syscall(91, fd, mode);
}

int fstat(int fd, struct stat *buf) {
    return syscall(5, fd, buf);
}

int lstat(const char *path, struct stat *buf) {
    return syscall(6, path, buf);
}

int mkdir(const char *pathname, mode_t mode) {
    return syscall(83, pathname, mode);
}

int mkfifo(const char *pathname, mode_t mode) {
    return -1;
}

int mknod(const char *pathname, mode_t mode, dev_t dev) {
    return syscall(133, pathname, mode, dev);
}

int stat(const char *path, struct stat *buf) {
    return syscall(4, path, buf);
}

mode_t umask(mode_t mask) {
    return syscall(95, mask);
}
