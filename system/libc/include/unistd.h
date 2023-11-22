#ifndef UNISTDC_H
#define UNISTDC_H

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#include <stddef.h>

int syscall(int, ...);

#endif //UNISTDC_H
