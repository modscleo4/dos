#include <dirent.h>

#include <unistd.h>

int closedir(DIR *dirp) {
    return 1;
}

DIR *opendir(const char *name) {
    return NULL;
}

struct dirent *readdir(DIR *dirp) {
    return NULL;
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
