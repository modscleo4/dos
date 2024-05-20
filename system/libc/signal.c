#include <signal.h>
#include <unistd.h>

int kill(int pid, int sig) {
    return syscall(62, pid, sig);
}

int raise(int sig) {
    return kill(getpid(), sig);
}

sighandler_t signal(int sig, sighandler_t handler) {
    return (sighandler_t) syscall(13, sig, handler);
}
