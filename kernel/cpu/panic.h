#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include <stdarg.h>
#include "system.h"

#define panic(msg, ...) _panic(__FILE__, __LINE__, msg, ##__VA_ARGS__)

void _panic(const char *file, const int line, const char *msg, ...);

void panic_handler(registers *r, const char *msg, ...);

#endif // KERNEL_PANIC_H
