#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include <stddef.h>

void dbgprint(const char *msg, ...);

void hexdump(void *ptr, size_t n);

void callstack(unsigned long int);

#endif // DEBUG_H
