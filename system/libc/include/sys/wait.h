#ifndef SYS_WAIT_H
#define SYS_WAIT_H

#include <sys/types.h>

pid_t wait(int *status);

pid_t waitpid(pid_t pid, int *status, int options);

#endif // SYS_WAIT_H
