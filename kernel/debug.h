#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

void dbgprint(const char *msg, ...);

void dbgwait(void);

void hexdump(void *ptr, size_t n);

void callstack(uint32_t ebp);

#endif // DEBUG_H
