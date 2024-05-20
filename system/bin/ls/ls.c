#include "ls.h"

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
    DIR *d;
    if (argc == 1) {
        d = opendir(".");
    } else {
        printf("ls: %d, %s\n", argc, argv[1]);
        d = opendir(argv[1]);
    }

    if (d == NULL) {
        printf("ls: failed to open directory\n");
        return 1;
    }

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        printf("%s\n", ent->d_name);
    }

    closedir(d);

    return 0;
}
