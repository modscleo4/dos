#ifndef PANIC_H
#define PANIC_H

#include "system.h"
#include <stdarg.h>

void panic(const char *msg, ...);

void panic_handler(const char *msg, registers *r);

#endif // PANIC_H
