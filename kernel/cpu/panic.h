#ifndef PANIC_H
#define PANIC_H

#include "system.h"
#include <stdarg.h>

void panic(const char *, ...);

void panic_handler(const char *, registers *);

#endif // PANIC_H
