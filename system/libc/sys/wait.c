#include <sys/wait.h>

#include <unistd.h>

pid_t wait(int *status) {
    return waitpid(-1, status, 0);
}

pid_t waitpid(pid_t pid, int *status, int options) {
    return syscall(61, pid, status, options);
}
