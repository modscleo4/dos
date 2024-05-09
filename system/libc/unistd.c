#include <unistd.h>
#include <stdarg.h>

int syscall(int sysno, ...) {
    va_list args;

    long int arg0, arg1, arg2, arg3, arg4, arg5;

    va_start(args, sysno);
    arg0 = va_arg(args, long int);
    arg1 = va_arg(args, long int);
    arg2 = va_arg(args, long int);
    arg3 = va_arg(args, long int);
    arg4 = va_arg(args, long int);
    arg5 = va_arg(args, long int);
    va_end(args);

    int retval;

    asm volatile("push %%ebp;"
        "mov %1, %%ebp;"
        "int $0x80;"
        "pop %%ebp"
        : "=g"(retval)
        : "g"(arg5), "a"(sysno), "b"(arg0), "c"(arg1), "d"(arg2), "S"(arg3), "D"(arg4)
        : "memory");

    return retval;
}

ssize_t read(int fd, void *buf, size_t count) {
    return syscall(0, fd, buf, count);
}

ssize_t write(int fd, void *buf, size_t count) {
    return syscall(1, fd, buf, count);
}

int close(int fd) {
    return syscall(3, fd);
}

pid_t getpid(void) {
    return syscall(39);
}

pid_t fork(void) {
    return syscall(57);
}

int execve(const char *filename, char *const argv[], char *const envp[]) {
    return syscall(59, filename, argv, envp);
}

char *getcwd(char *buf, size_t size) {
    return (char *)syscall(79, buf, size);
}

int chdir(const char *path) {
    return syscall(80, path);
}

int fchdir(int fd) {
    return syscall(81, fd);
}

uid_t getuid(void) {
    return syscall(102);
}

gid_t getgid(void) {
    return syscall(104);
}

int setuid(uid_t uid) {
    return syscall(105, uid);
}

int setgid(gid_t gid) {
    return syscall(106, gid);
}

uid_t geteuid(void) {
    return syscall(107);
}

gid_t getegid(void) {
    return syscall(108);
}

pid_t getppid(void) {
    return syscall(110);
}
