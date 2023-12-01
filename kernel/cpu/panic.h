#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include <stdarg.h>
#include "system.h"

void panic(const char *msg, ...);

void panic_handler(registers *r, const char *msg, ...);

#endif // KERNEL_PANIC_H
