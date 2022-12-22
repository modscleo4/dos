#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include <stdarg.h>
#include "system.h"

void panic(const char *msg, ...);

void panic_handler(const char *msg, registers *r);

#endif // KERNEL_PANIC_H
