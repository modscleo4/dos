#include "init.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    open("/dev/tty1", O_RDONLY); // stdin
    open("/dev/tty1", O_WRONLY); // stdout
    open("/dev/tty1", O_WRONLY); // stderr

    int fstab_fd = open("/etc/fstab", O_RDONLY);
    if (fstab_fd < 0) {
        printf("failed to open /etc/fstab: %d\n", fstab_fd);
        return 1;
    }

    char buf[1024];
    size_t to_read = 64;
    for (int i = 0; i < 1024; i += to_read) {
        ssize_t l = read(fstab_fd, buf + i, to_read);
        if (l < 0) {
            printf("failed to read /etc/fstab: %ld\n", l);
            return 1;
        }

        if (l < to_read) {
            buf[i + l] = '\0';
            break;
        }
    }

    buf[1023] = '\0';

    close(fstab_fd);

    printf("init started with %d arguments\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    printf("init reading /etc/fstab:\n%s\n", buf);

    pid_t pid = fork();
    if (pid == 0) {
        pid = getpid();
        printf("I am the child, my PID is %ld\n", pid);

        char *exe = "/bin/sh.elf";
        char *argv[] = {"sh.elf", NULL};
        char *envp[] = {"PATH=/bin", NULL};

        printf("executing %s\n", exe);

        int r;
        if ((r = execve(exe, argv, envp))) {
            printf("exec failed: %d\n", r);
            return 1;
        }
    } else {
        printf("I am the parent, child PID is %ld\n", pid);
        printf("waiting for child to exit\n");
        int status;
        waitpid(pid, &status, 0);
        printf("child exited with status %d\n", status);
    }

    return 0;
}
