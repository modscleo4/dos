#ifndef UNISTDC_H
#define UNISTDC_H

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#include <stddef.h>
#include <sys/types.h>

int syscall(int, ...);

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, void *buf, size_t count);
int close(int fd);
pid_t getpid(void);
pid_t getppid(void);
pid_t fork(void);
int execve(const char *filename, char *const argv[], char *const envp[]);
char *getcwd(char *buf, size_t size);
int chdir(const char *path);
int fchdir(int fd);
uid_t getuid(void);
gid_t getgid(void);
int setuid(uid_t uid);
int setgid(gid_t gid);
uid_t geteuid(void);
gid_t getegid(void);

#endif //UNISTDC_H
