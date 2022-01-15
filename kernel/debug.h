#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include <stddef.h>

void dbgprint(const char *msg, ...);

void hexdump(void *ptr, size_t n);

#endif // DEBUG_H
