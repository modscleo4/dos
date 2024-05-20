#include <dirent.h>

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

struct DIR {
    int fd;
};

int closedir(DIR *dirp) {
    if (dirp == NULL) {
        return -1;
    }

    close(dirp->fd);
    free(dirp);
    return 0;
}

DIR *opendir(const char *name) {
    int fd = open(name, O_RDONLY | O_DIRECTORY);
    if (fd < 0) {
        return NULL;
    }

    DIR *dirp = (DIR *)malloc(sizeof(DIR));
    dirp->fd = fd;
    return dirp;
}

struct dirent *readdir(DIR *dirp) {
    if (dirp == NULL) {
        return NULL;
    }

    static struct dirent entry;
    int ret = read(dirp->fd, &entry, sizeof(struct dirent));
    if (ret <= 0) {
        return NULL;
    }

    return &entry;
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
    return 1;
}

void rewinddir(DIR *dirp) {

}

void seekdir(DIR *dirp, long int loc) {

}

long telldir(DIR *dirp) {
    return 0;
}
