#include "cat.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s <file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf("failed to open %s: %d\n", argv[1], fd);
        return 1;
    }

    char buf[1024];
    size_t to_read = 64;
    for (int i = 0; i < 1024; i += to_read) {
        ssize_t l = read(fd, buf + i, to_read);
        if (l < 0) {
            printf("failed to read %s: %ld\n", argv[1], l);
            return 1;
        }

        if (l < to_read) {
            buf[i + l] = '\0';
            break;
        }
    }

    buf[1023] = '\0';

    close(fd);

    return 0;
}
