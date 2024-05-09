#include <fcntl.h>

#include <unistd.h>

int open(const char *pathname, int flags) {
    return syscall(2, pathname, flags);
}
