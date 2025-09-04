#include "sh.h"

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_ARGS 10

void sig_handler(int sig) {
    printf("Signal %x received\n", sig);
}

int main(int argc, char *argv[]) {
    printf("sh started with %d arguments, UID=%ld, GID=%ld\n", argc, getuid(), getgid());
    for (int i = 0; i < argc; i++) {
        printf("arg %d: %p=%s\n", i, argv[i], argv[i]);
    }

    for (int i = 0; i < 32; i++) {
        if (signal(i, sig_handler) > 0) {
            printf("Registered signal handler for signal %d\n", i);
        }
    }

    char str[1024];
    while (true) {
        printf("> ");
        fgets(str, 1024, stdin);

        if (strlen(str) == 1) {
            continue;
        }

        // Remove newline
        str[strlen(str) - 1] = '\0';

        if (strcmp(str, "exit") == 0) {
            break;
        } else if (strcmp(str, "div0") == 0) {
            printf("%f\n", 0.0F / .0F);
            printf("%f\n", 1.0F / .0F);
            printf("%f\n", -1.0F / .0F);
            printf("%f\n", -1.0F);
        } else if (strcmp(str, "gpf") == 0) {
            asm volatile("hlt");
        } else if (strcmp(str, "panic") == 0) {
            syscall(127, "PANIC Test from ring3\n");
        } else if (strncmp(str, "while", 5) == 0) {
            long int i = 0;
            while (i < 0x7FFFFFFE) {
                i++;
            }
        } else if (strncmp(str, "nyx", 3) == 0) {
            int fd = open("/etc/nyx.txt", O_RDONLY);
            if (fd < 0) {
                printf("open failed: %d\n", fd);
                continue;
            }

            struct stat st;
            if (fstat(fd, &st) < 0) {
                printf("fstat failed\n");
                close(fd);
                continue;
            }

            char *buf = malloc(st.st_blksize);
            if (buf == NULL) {
                printf("malloc failed\n");
                close(fd);
                continue;
            }

            int r;
            while ((r = read(fd, buf, st.st_blksize)) > 0) {
                write(1, buf, r);
            }

            if (r < 0) {
                printf("read failed: %d\n", r);
            }

            close(fd);
            free(buf);
        } else if (strncmp(str, "fork", 4) == 0) {
            pid_t pid = fork();
            if (pid == 0) {
                printf("I am the child, PID is %ld, parent PID is %ld\n", getpid(), getppid());
            } else {
                printf("I am the parent, PID is %ld\n", pid);
            }
        } else if (strncmp(str, "exec ", 5) == 0) {
            char *exe = str + 5;
            char *argv[] = {NULL};
            char *envp[] = {NULL};

            int r;
            if ((r = execve(exe, argv, envp))) {
                printf("exec failed: %d\n", r);
            }
        } else if (strncmp(str, "signal ", 7) == 0) {
            int pid = atoi(str + 7);
            int sig = 9;
            int r;
            printf("Sending signal %d to PID %d\n", sig, pid);
            if ((r = kill(pid, sig))) {
                printf("kill failed: %d\n", r);
            }
        } else {
            char exe[256] = "";
            // Copy command to exe up to first space
            int i = 0;
            while (str[i] != ' ' && str[i] != '\0' && i < 255) {
                exe[i] = str[i];
                i++;
            }

            char tmp[1024] = "";
            if (exe[0] != '/') {
                sprintf(tmp, "/bin/%s.elf", exe);
            } else {
                sprintf(tmp, "%s.elf", exe);
            }

            int ret;
            if ((ret = stat(tmp, NULL)) == 0) {
                char *argv[MAX_ARGS] = {NULL};
                char *envp[] = {"PATH=/bin", NULL};

                char *token = strtok(str, " ");
                int i = 0;
                while (token != NULL && i < MAX_ARGS) {
                    if (!strlen(token)) {
                        goto next;
                    }

                    argv[i++] = token;
                next:
                    token = strtok(NULL, " ");
                }

                pid_t pid = fork();
                if (pid == 0) {
                    int r;
                    if ((r = execve(tmp, argv, envp))) {
                        printf("exec failed: %d\n", r);
                    }
                } else {
                    int status;
                    waitpid(pid, &status, 0);
                    printf("$%d ", status);
                }
            } else {
                printf("%s: unknown command (%d)\n", exe, ret);
            }
        }
    }

    return 0;
}
