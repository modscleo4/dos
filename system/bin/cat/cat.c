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

    struct stat st;
    if (fstat(fd, &st) < 0) {
        printf("failed to stat %s: %d\n", argv[1], errno);
        close(fd);
        return 1;
    }

    char buf[1025];
    for (int i = 0; i < st.st_size; i += 1024) {
        ssize_t l = read(fd, buf, 1024);
        if (l < 0) {
            printf("failed to read %s: %ld\n", argv[1], l);
            return 1;
        }

        printf("%.*s", (int)l, buf);
    }

    close(fd);

    return 0;
}
